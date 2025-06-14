import cv2
import numpy as np
import csv
import math
import pyrealsense2 as rs
from typing import Tuple, Optional, List


class RealSenseCamera:
    """RealSense相機控制類"""
    
    def __init__(self, width: int = 1920, height: int = 1080, fps: int = 30):
        self.pipeline = rs.pipeline()
        self.config = rs.config()
        self.config.enable_stream(rs.stream.color, width, height, rs.format.bgr8, fps)
        
    def start(self, exposure: int = 300, gain: int = 16):
        """啟動相機並設置參數"""
        profile = self.pipeline.start(self.config)
        sensor = self.pipeline.get_active_profile().get_device().first_color_sensor()
        sensor.set_option(rs.option.exposure, exposure)
        sensor.set_option(rs.option.gain, gain)
        
    def capture_frame(self) -> np.ndarray:
        """捕獲一幀圖像"""
        frames = self.pipeline.wait_for_frames()
        color_frame = frames.get_color_frame()
        return np.asanyarray(color_frame.get_data())
    
    def stop(self):
        """停止相機"""
        self.pipeline.stop()


class ImageProcessor:
    """圖像處理類"""
    
    @staticmethod
    def resize_image(image: np.ndarray, scale: float = 0.5) -> np.ndarray:
        """調整圖像大小"""
        return cv2.resize(image, (0, 0), fx=scale, fy=scale)
    
    @staticmethod
    def adjust_brightness_contrast(image: np.ndarray, contrast: int = 600, brightness: int = 100) -> np.ndarray:
        """調整圖像亮度和對比度"""
        output = image * (contrast / 127 + 1) - contrast + brightness
        output = np.clip(output, 0, 255)
        return np.uint8(output)
    
    @staticmethod
    def preprocess_for_circle_detection(image: np.ndarray) -> np.ndarray:
        """圖像預處理用於圓形檢測"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        return cv2.GaussianBlur(gray, (5, 5), 0)


class BallDetector:
    """球體檢測類"""
    
    def __init__(self, table_bounds: Tuple[int, int, int, int] = (100, 395, 150, 865)):
        """
        初始化球體檢測器
        table_bounds: (y_min, y_max, x_min, x_max) 桌面邊界
        """
        self.table_bounds = table_bounds
    
    def detect_circles(self, gray_image: np.ndarray) -> Optional[np.ndarray]:
        """檢測圓形"""
        circles = cv2.HoughCircles(
            gray_image, 
            cv2.HOUGH_GRADIENT, 
            1, 20, None, 100, 15, 10, 18
        )
        return circles.astype(int) if circles is not None else None
    
    def is_within_table(self, x: int, y: int) -> bool:
        """檢查座標是否在桌面範圍內"""
        y_min, y_max, x_min, x_max = self.table_bounds
        return y_min <= y <= y_max and x_min <= x <= x_max
    
    def is_white_ball(self, image: np.ndarray, x: int, y: int) -> bool:
        """檢測是否為白球（母球）"""
        for h in range(5, 10):
            positions = [(y+h, x+h), (y-h, x-h), (y+h, x-h), (y-h, x+h)]
            for py, px in positions:
                if not all(image[py, px] > 253):
                    return False
        return True
    
    def detect_balls(self, image: np.ndarray, circles: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        檢測母球和子球
        返回: (mother_ball, child_balls)
        """
        mother_ball = None
        child_balls = []
        
        if circles is not None and len(circles) > 0:
            for x, y, r in circles[0]:
                if not self.is_within_table(x, y):
                    continue
                
                if self.is_white_ball(image, x, y):
                    mother_ball = np.array([x, y])
                else:
                    child_balls.append([x, y])
        
        return mother_ball, np.array(child_balls) if child_balls else np.array([])
    
    def filter_child_balls(self, mother_ball: np.ndarray, child_balls: np.ndarray) -> np.ndarray:
        """過濾掉與母球重疊的子球"""
        if mother_ball is None or len(child_balls) == 0:
            return child_balls
        
        filtered_balls = []
        for ball in child_balls:
            if abs(mother_ball[0] - ball[0]) > 1 and abs(mother_ball[1] - ball[1]) > 1:
                filtered_balls.append(ball)
        
        return np.array(filtered_balls)


class CoordinateTransformer:
    """座標轉換類"""
    
    def __init__(self, intrinsic_path: str, rotation_path: str, translation_path: str):
        self.intrinsic_matrix = self._load_matrix(intrinsic_path)
        self.rotation_matrix = self._load_matrix(rotation_path)
        self.translation_vector = self._load_matrix(translation_path)
        self.extrinsic_matrix = self._build_extrinsic_matrix()
        self.homography_matrix = self._build_homography_matrix()
        self.homography_inv = np.linalg.inv(self.homography_matrix)
    
    def _load_matrix(self, file_path: str) -> np.ndarray:
        """從CSV文件加載矩陣"""
        with open(file_path, 'r') as f:
            data = list(csv.reader(f, delimiter=","))
        return np.array(data).astype(float)
    
    def _build_extrinsic_matrix(self) -> np.ndarray:
        """構建外參矩陣"""
        extrinsic = np.zeros((3, 3))
        extrinsic[:3, :3] = self.rotation_matrix
        extrinsic[0, 2] = self.translation_vector[0, 0]
        extrinsic[1, 2] = self.translation_vector[0, 1]
        extrinsic[2, 2] = self.translation_vector[0, 2]
        return extrinsic.astype(float)
    
    def _build_homography_matrix(self) -> np.ndarray:
        """構建單應性矩陣"""
        return np.matmul(self.intrinsic_matrix, self.extrinsic_matrix)
    
    def _scale_factor(self, u: int, v: int) -> float:
        """計算尺度因子"""
        return (self.homography_inv[2, 0] * u + 
                self.homography_inv[2, 1] * v + 
                self.homography_inv[2, 2])
    
    def pixel_to_world(self, pixel_coords: np.ndarray, scale: int = 2) -> np.ndarray:
        """將像素座標轉換為世界座標"""
        # 恢復原始尺寸
        scaled_coords = pixel_coords * scale
        
        if scaled_coords.ndim == 1:
            # 單個點
            homogeneous = np.append(scaled_coords, 1)
            world_coords = np.matmul(self.homography_inv, homogeneous.T) / self._scale_factor(scaled_coords[0], scaled_coords[1])
            return world_coords[:2]  # 只返回x, y座標
        else:
            # 多個點
            world_coords = []
            for coord in scaled_coords:
                homogeneous = np.append(coord, 1)
                world_coord = np.matmul(self.homography_inv, homogeneous.T) / self._scale_factor(coord[0], coord[1])
                world_coords.append(world_coord[:2])
            return np.array(world_coords)


class DataManager:
    """數據管理類"""
    
    @staticmethod
    def save_to_csv(data: np.ndarray, filename: str, format_type: str = '%f'):
        """保存數據到CSV文件"""
        np.savetxt(filename, data, delimiter=',', fmt=format_type)
    
    @staticmethod
    def load_test_points(file_path: str) -> np.ndarray:
        """加載測試點"""
        with open(file_path, 'r') as f:
            data = list(csv.reader(f, delimiter=","))
        return np.array(data).astype(float)


class Visualizer:
    """可視化類"""
    
    @staticmethod
    def draw_circles(image: np.ndarray, circles: np.ndarray, radius: int, color: Tuple[int, int, int] = (255, 0, 0), thickness: int = 2):
        """在圖像上繪製圓圈"""
        for circle in circles:
            if len(circle) >= 2:
                cv2.circle(image, (int(circle[0]), int(circle[1])), radius, color, thickness)
    
    @staticmethod
    def draw_mother_ball(image: np.ndarray, mother_ball: np.ndarray, radius: int, color: Tuple[int, int, int] = (0, 0, 255), thickness: int = 2):
        """繪製母球"""
        if mother_ball is not None:
            cv2.circle(image, (int(mother_ball[0]), int(mother_ball[1])), radius, color, thickness)
    
    @staticmethod
    def draw_child_balls(image: np.ndarray, child_balls: np.ndarray, radius: int, color: Tuple[int, int, int] = (255, 255, 255), thickness: int = 2):
        """繪製子球"""
        if len(child_balls) > 0:
            Visualizer.draw_circles(image, child_balls, radius, color, thickness)
    
    @staticmethod
    def show_image(image: np.ndarray, window_name: str = 'Image'):
        """顯示圖像"""
        cv2.imshow(window_name, image)
        cv2.waitKey(0)
        cv2.destroyAllWindows()


class BilliardDetectionSystem:
    """撞球檢測系統主類"""
    
    def __init__(self, intrinsic_path: str, rotation_path: str, translation_path: str):
        self.image_processor = ImageProcessor()
        self.ball_detector = BallDetector()
        self.coordinate_transformer = CoordinateTransformer(intrinsic_path, rotation_path, translation_path)
        self.data_manager = DataManager()
        self.visualizer = Visualizer()
    
    def process_image_from_file(self, image_path: str) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        """從文件處理圖像"""
        # 讀取和預處理圖像
        original = cv2.imread(image_path)
        resized = self.image_processor.resize_image(original)
        adjusted = self.image_processor.adjust_brightness_contrast(resized)
        gray = self.image_processor.preprocess_for_circle_detection(adjusted)
        
        # 檢測圓圈
        circles = self.ball_detector.detect_circles(gray)
        
        # 檢測球體
        mother_ball, child_balls = self.ball_detector.detect_balls(adjusted, circles)
        filtered_child_balls = self.ball_detector.filter_child_balls(mother_ball, child_balls)
        
        # 可視化結果
        result_image = adjusted.copy()
        if mother_ball is not None:
            self.visualizer.draw_mother_ball(result_image, mother_ball, 15)
        if len(filtered_child_balls) > 0:
            self.visualizer.draw_child_balls(result_image, filtered_child_balls, 15)
        
        return mother_ball, filtered_child_balls, result_image
    
    def convert_to_world_coordinates(self, mother_ball: np.ndarray, child_balls: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """轉換為世界座標"""
        world_mother = None
        world_children = None
        
        if mother_ball is not None:
            world_mother = self.coordinate_transformer.pixel_to_world(mother_ball)
        
        if len(child_balls) > 0:
            world_children = self.coordinate_transformer.pixel_to_world(child_balls)
        
        return world_mother, world_children
    
    def save_results(self, world_mother: np.ndarray, world_children: np.ndarray, num_children: int):
        """保存結果到CSV文件"""
        if world_mother is not None:
            mother_2d = np.array([world_mother])
            self.data_manager.save_to_csv(mother_2d, 'mother_Wcoor.csv')
        
        if world_children is not None:
            self.data_manager.save_to_csv(world_children, 'son_Wcoor.csv')
        
        num_array = np.array([[num_children]])
        self.data_manager.save_to_csv(num_array, 'childball_num.csv')
    
    def process_test_points(self, test_points_path: str, test_image_path: str):
        """處理測試點"""
        test_points = self.data_manager.load_test_points(test_points_path)
        
        # 在測試圖像上繪製測試點
        test_image = cv2.imread(test_image_path)
        for point in test_points:
            cv2.circle(test_image, (int(point[0]), int(point[1])), 2, (0, 0, 255), 2)
        
        self.visualizer.show_image(test_image, 'Test Points')
        
        # 轉換測試點到世界座標
        world_test_points = self.coordinate_transformer.pixel_to_world(test_points, scale=1)
        
        # 保存測試結果
        self.data_manager.save_to_csv(world_test_points, 'test_points.csv')
        test_num = np.array([[len(test_points)]])
        self.data_manager.save_to_csv(test_num, 'test_num.csv')


def main():
    """主函數示例"""
    # 設置文件路徑
    intrinsic_path = "D:/matlab/hiwin_robotic_arm/intrinsic_matrix33"
    rotation_path = "D:/matlab/hiwin_robotic_arm/extrinsic_matrix33"
    translation_path = "D:/matlab/hiwin_robotic_arm/translation_vector13"
    image_path = "realsense_colorimg.jpg"
    test_points_path = "D:/matlab/hiwin_robotic_arm/picpoint"
    test_image_path = "C:/Users/wang8/OneDrive/桌面/機械手臂相機/real_sense/41_Color.png"
    
    # 創建檢測系統
    detection_system = BilliardDetectionSystem(intrinsic_path, rotation_path, translation_path)
    
    # 處理圖像
    mother_ball, child_balls, result_image = detection_system.process_image_from_file(image_path)
    
    # 顯示結果
    detection_system.visualizer.show_image(result_image, 'Detection Result')
    
    # 轉換為世界座標
    world_mother, world_children = detection_system.convert_to_world_coordinates(mother_ball, child_balls)
    
    # 保存結果
    num_children = len(child_balls) if child_balls is not None else 0
    detection_system.save_results(world_mother, world_children, num_children)
    
    # 處理測試點
    detection_system.process_test_points(test_points_path, test_image_path)
    
    # 打印結果
    print('母球:', world_mother)
    print('子球:', world_children)
    print('----------------------------------------')


if __name__ == "__main__":
    main()