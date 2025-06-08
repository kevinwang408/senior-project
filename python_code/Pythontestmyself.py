import cv2
import numpy as np
import csv
import math
import pyrealsense2 as rs


# 配置RealSense相机

'''

pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.color, 1920, 1080, rs.format.bgr8, 30)
profile = pipeline.start(config)

# 设置曝光时间和增益

sensor = pipeline.get_active_profile().get_device().first_color_sensor()
sensor.set_option(rs.option.exposure, 300) # 设置曝光时间为1000微秒
sensor.set_option(rs.option.gain, 16) # 设置增益为16倍


frames = pipeline.wait_for_frames()
color_frame = frames.get_color_frame()
image_np = np.asanyarray(color_frame.get_data()) # 彩色圖
cv2.imwrite('realsense_colorimg.jpg',image_np) #輸出圖片
'''

original=cv2.imread(r'realsense_colorimg.jpg')
img = cv2.resize(original,(0,0),fx=0.5,fy=0.5)
cv2.imshow('frame',img)
cv2.waitKey(0)
cv2.destroyAllWindows()

contrast =600#230good  2023_0410_235 20230423 550 
brightness =100#380  20230410_180  20230423 120
output = img * (contrast/127 + 1) - contrast + brightness#家亮度
#print(output"C:\Users\wang8\OneDrive\桌面\機械手臂相機\ball\balltest3_8-5.bmp"
output = np.clip(output, 0, 255)#擷取於0~255
#print(output)
output = np.uint8(output)#轉整數
#print(output)
cv2.imshow('frame',output)
cv2.waitKey(0)
cv2.destroyAllWindows()
gray = cv2.cvtColor(output, cv2.COLOR_BGR2GRAY)
gray = cv2.GaussianBlur(gray,(5, 5),0)
circles = cv2.HoughCircles(gray, cv2.HOUGH_GRADIENT, 1, 20, None, 100, 15, 10, 18)#檢測子球位置(x,y,r)
print(circles)
circles = circles.astype(int)#子球位置整數化
print(circles)
son = np.array([])
#print(circles[0])

if len(circles) > 0:#
    for x, y, r in circles[0]:#x y r在求座標內
        #if(y<443 or y>1177 or x<175 or x>1596):#100 400 100 750
        if(y<100 or y>395 or x<150 or x>865):#140
            #b = np.where(circles[0] == [x,y,r])
            #c = np.append(c, b)

            continue#若是在桌外則跳出此輪繼續執行迴圈
        cv2.circle(output, (x, y), r ,(255,0,0),2)#畫出圓圈 位置 半徑 顏色 圓類型
        son = np.append(son,[x,y])
        for h in range(5,10):
            if(output[y+h,x+h][0]>253 and output[y+h,x+h][1]>253 and output[y+h,x+h][2]>253 and output[y-h,x-h][0]>253 and output[y-h,x-h][1]>253 and output[y-h,x-h][2]>253 and output[y+h,x-h][0]>253 and output[y+h,x-h][1]>253 and output[y+h,x-h][2]>253 and output[y-h,x+h][0]>253 and output[y-h,x+h][1]>253 and output[y-h,x+h][2]>253):
                cv2.circle(output, (x, y), r ,(0,0,255),2)
                mother = np.array([x,y])
                print(mother)



son = son.astype(int)
son = son.reshape(int(len(son)/2),-1)#-1為自動設定 將陣列轉成len(son)除以2數量的列 行隨機
sons=np.array([])
print("檢查一")
#print(len(sons))
mother=mother.astype(int)
print(mother[0])
print(mother[1])
#print(son[1][0])
#print(son[1][1])

i=0
test1=0
test2=100
for i in range(0,len(son)):
    test1=mother[0]-son[i][0]
    test2=mother[1]-son[i][0]
    if abs(test1)>1:
        if abs(test2)>1:
            sons=np.append(sons,son[i][0])
            sons=np.append(sons,son[i][1])


sons=sons.astype(int)
sons = sons.reshape(int(len(sons)/2),-1)
#e = np.where(son == mother[0])
#son = np.delete(son, e)
#f = np.where(son == mother[1])#mother x and y
#son = np.delete(son, f)
i=0
print("檢查")
print(sons)
#sons=sons.astype(int)
#cv2.circle(output, (son[0][0], son[0][1]), r ,(255,255,255),2)



#print(son)
#print(mother)

#print(sons)#檢測出son之型態 應為2組(x y)座標為一格
#print(sons.shape)

print(sons[0][0])
print(sons[0][1])

cv2.circle(output, (sons[0][0], sons[0][1]), r ,(255,255,255),2)

cv2.imshow('circle out',output)
cv2.waitKey(0)
cv2.destroyAllWindows()

j=0
for i in sons:#計算求個數
    j+=1
childball_num=np.zeros((1,1),int)
childball_num[0]=j#將子球個數存成一個num
#print(childball_num)
#print(sons)
for i in range(0,j):#因為畫面縮小成1/2，座標要乘2
    sons[i][0]*=2
    sons[i][1]*=2
#print(sons)
for i in range(0,2):
    mother[i]*=2


cueball=np.zeros((1,2),int)
cueball[0]=mother#轉二維方便csv讀取
print(cueball)

# np.savetxt('pixel_childball.csv', sons, delimiter=',',fmt='%d')
# np.savetxt('pixel_cueball.csv',cueball, delimiter=',',fmt='%d')
# np.savetxt('childball_num.csv', childball_num, delimiter=',',fmt='%d')

#print(mother)


#cv2.imshow('samuelbitchmotherfuck',output)
#cv2.waitKey(0)
#cv2.destroyAllWindows()
print('母球:')
print(mother)
print('子球')
print(sons)
print('----------------------------------------')

with open("D:/matlab/hiwin_robotic_arm/intrinsic_matrix33", 'r') as Intrinsic_csv:#csv內參讀取
     Intrinsic_csv = list(csv.reader(Intrinsic_csv, delimiter=","))
with open("D:/matlab/hiwin_robotic_arm/translation_vector13", 'r') as TranslationVector_csv:
     TranslationVector_csv = list(csv.reader(TranslationVector_csv, delimiter=","))
with open("D:/matlab/hiwin_robotic_arm/extrinsic_matrix33", 'r') as RotationMatrix_csv:
     RotationMatrix_csv = list(csv.reader(RotationMatrix_csv, delimiter=","))
print(Intrinsic_csv)
Intrinsic = np.array(Intrinsic_csv)
RotationMatrix = np.array(RotationMatrix_csv)
TranslationVector= np.array(TranslationVector_csv)
#print(Intrinsic)
#print(RotationMatrix)
#print(TranslationVector)
ExtrinsicMatrix=np.zeros((3,3))
#hrtimesthrprolong=[[0],[0],[0]]#3*3化成3*4
#thrtimesfourprolong=[[0,0,0,1]]#3*4化成4*4
#Intrinsic=np.append(Intrinsic,thrtimesthrprolong,axis=1)#3*4
#Intrinsic=np.append(Intrinsic,thrtimesfourprolong,axis=0)#4*4
#做出外參
ExtrinsicMatrix[0][0]=RotationMatrix[0][0]
ExtrinsicMatrix[0][1]=RotationMatrix[0][1]
ExtrinsicMatrix[0][2]=RotationMatrix[0][2]
ExtrinsicMatrix[1][0]=RotationMatrix[1][0]
ExtrinsicMatrix[1][1]=RotationMatrix[1][1]
ExtrinsicMatrix[1][2]=RotationMatrix[1][2]
ExtrinsicMatrix[2][0]=RotationMatrix[2][0]
ExtrinsicMatrix[2][1]=RotationMatrix[2][1]
ExtrinsicMatrix[2][2]=RotationMatrix[2][2]
ExtrinsicMatrix=ExtrinsicMatrix.astype(float)
theta=2.725*math.pi/180
'''
z_rotate=np.array([[math.cos(theta),-math.sin(theta),0],
                   [math.sin(theta),math.cos(theta),0],
                   [0,0,1]]);
theta2=6*math.pi/180
x_rotate=np.array([[1,0,0],
                  [0,math.cos(theta2),-math.sin(theta2)],
                  [0,math.sin(theta2),math.cos(theta2)]])
ExtrinsicMatrix=np.matmul(ExtrinsicMatrix,z_rotate)
ExtrinsicMatrix=np.matmul(ExtrinsicMatrix,x_rotate)
'''
ExtrinsicMatrix[0][2]=TranslationVector[0][0]
ExtrinsicMatrix[1][2]=TranslationVector[0][1]
ExtrinsicMatrix[2][2]=TranslationVector[0][2]
#ExtrinsicMatrix=np.append(ExtrinsicMatrix,thrtimesfourprolong,axis=0)#4*4
ExtrinsicMatrix=ExtrinsicMatrix.astype(float)

Intrinsic=Intrinsic.astype(float)
#print(Intrinsic)
#print(ExtrinsicMatrix)
#Zc=ExtrinsicMatrix[2][3]#Zc
#matrix_H=Intrinsic*ExtrinsicMatrix

matrix_H=np.matmul(Intrinsic,ExtrinsicMatrix)
# print(Intrinsic)
# print(ExtrinsicMatrix)
#print(matrix_H)
# cv2.waitKey(0)
# cv2.destroyAllWindows()
matrix_H_inv=np.linalg.inv(matrix_H)#返乘矩陣
#print(matrix_H_inv)
def s(u,v):
    s=matrix_H_inv[2][0]*u+matrix_H_inv[2][1]*v+matrix_H_inv[2][2] #尺度因子 為Zc分之1
    return s
mother=np.insert(mother,2,1)
mother_T=(np.array([mother])).T
mother_Wcoor=(np.matmul(matrix_H_inv,mother_T)/s(mother[0],mother[1])).T#除Zc分之一 及為乘上Zc
#print(mother_Wcoor)

i=0
childball=np.zeros((j,3))

for son in sons:
    son=np.insert(son,2,1)
    childball[i]=son
    i+=1
#print("可能子球")
#print(childball)

son_Wcoor=np.zeros((j,3))
i=0
for ball in childball:
    son_Wcoor[i]=np.matmul(matrix_H_inv,ball.T)/s(ball[0],ball[1])
    i+=1

i=0
son_Wcoor_normalize=np.zeros((j,2))
for son in son_Wcoor:    
    son=np.delete(son,2)
    son_Wcoor_normalize[i]=son
    i+=1
#print(son_Wcoor_normalize)

mother_Wcoor=np.delete(mother_Wcoor,2)
mother_Wcoor=np.array([mother_Wcoor])
#print(mother_Wcoor)


np.savetxt('son_Wcoor.csv', son_Wcoor_normalize, delimiter=',',fmt='%f')#存成float
np.savetxt('mother_Wcoor.csv',mother_Wcoor, delimiter=',',fmt='%f')
np.savetxt('childball_num.csv', childball_num, delimiter=',',fmt='%f')

#################################################################################
#測試點
#################################################################################


with open(r"D:\matlab\hiwin_robotic_arm\picpoint", 'r') as test_point:#讀取取得資料之座標點
     test_point_csv = list(csv.reader(test_point, delimiter=","))
     test_point=np.array(test_point_csv)#此為字串
     test_point=test_point.astype(float)
     #print(test_point)

test=cv2.imread(r"C:\Users\wang8\OneDrive\桌面\機械手臂相機\real_sense\41_Color.png")
draw=test_point.astype(int)
for i in range(0,len(test_point)):
    cv2.circle(test, (draw[i][0], draw[i][1]), 2 ,(0,0,255),2)

cv2.imshow('circle out',test)
cv2.waitKey(0)
cv2.destroyAllWindows()

points_test=np.zeros((len(test_point),3))
print("長度")
print(len(test_point))
for i in range(0,len(test_point)):
    points_test[i][0]=test_point[i][0]
    points_test[i][1]=test_point[i][1]
    points_test[i][2]=1


def news(u,v,z):#自己定義 因為我是先*H-1
    s=matrix_H_inv[2][0]*u+matrix_H_inv[2][1]*v+matrix_H_inv[2][2]*z #尺度因子 為Zc分之1
    return s
#print("points_test[i]")
#print(points_test[1])

point_test_Wcoor=(np.matmul(matrix_H_inv,points_test.T))
print(point_test_Wcoor)
i=0
print(point_test_Wcoor)
for i in range(0,len(points_test)):
    point_test_Wcoor[0][i]=point_test_Wcoor[0][i]/ point_test_Wcoor[2][i]
    point_test_Wcoor[1][i]=point_test_Wcoor[1][i]/ point_test_Wcoor[2][i]
    point_test_Wcoor[2][i]=point_test_Wcoor[2][i]/ point_test_Wcoor[2][i]
print("here")
print(point_test_Wcoor.T)
#非我
i=0
for  point_test in points_test:
    points_test[i]=np.matmul(matrix_H_inv,point_test.T)/s(point_test[0],point_test[1])
    i+=1

#print(points_test)
#print(point_test)
test_num=np.zeros((1,0))
test_num[0]=len(points_test)






np.savetxt('test_num.csv', test_num, delimiter=',',fmt='%f')
np.savetxt('test_points.csv', points_test, delimiter=',',fmt='%f')

#print(point_test)
#test_point_Wcoor=(np.matmul(matrix_H_inv,point_test_T)/s(mother[0],mother[1])).T
# uvcoor=s(226,453)*np.matmul(matrix_H,[[0],[0],[1]])
# a=uvcoor[0][0]
# b=uvcoor[1][0]
# img = cv2.imread("C:/Users/samuel901213/Desktop/wow/1225.bmp")
# cv2.circle(img,(int(a),int(b)),1,(0,0,255),3)
# cv2.imshow('samuelbitchmotherfuck',img)
# cv2.waitKey(0)
# cv2.destroyAllWindows()