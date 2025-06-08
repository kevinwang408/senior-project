import cv2
import numpy as np
import csv
import math


original=cv2.imread(r"C:\Users\wang8\source\repos\C++candidate_path\C++candidate_path\realsense_colorimg.jpg")
output = cv2.resize(original,(0,0),fx=0.5,fy=0.5)

with open(r"C:\Users\wang8\source\repos\C++candidate_path\C++candidate_path\direct_collide.csv", 'r') as direct_collide:#讀取取得資料之座標點
     direct_collide_csv = list(csv.reader(direct_collide, delimiter=","))
     direct_collide=np.array(direct_collide_csv)#此為字串
     direct_collide=direct_collide.astype(float)
     #print(flip_once_pointplot)

with open(r"C:\Users\wang8\source\repos\C++candidate_path\C++candidate_path\circle.csv", 'r') as circle:#讀取取得資料之座標點
     circle_csv = list(csv.reader(circle, delimiter=","))
     circleplot=np.array(circle_csv)#此為字串
     circleplot=circleplot.astype(float)
     #print(circleplot)


with open(r"C:\Users\wang8\source\repos\C++candidate_path\C++candidate_path\flip_once_point.csv", 'r') as flip_once_point:#讀取取得資料之座標點
     flip_once_point_csv = list(csv.reader(flip_once_point, delimiter=","))
     flip_once_pointplot=np.array(flip_once_point_csv)#此為字串
     flip_once_pointplot=flip_once_pointplot.astype(float)
     #print(flip_once_pointplot)

with open(r"C:\Users\wang8\source\repos\C++candidate_path\C++candidate_path\final_coor.csv", 'r') as final_coor:#讀取取得資料之座標點
     final_coor_csv = list(csv.reader(final_coor, delimiter=","))
     final_coor=np.array(final_coor_csv)#此為字串
     final_coor=final_coor.astype(float)


final_coor=np.append(final_coor,1)
final_coor[2]=1
print(final_coor)
print(final_coor.T)

dircet_append=np.zeros((len(direct_collide),1))#做出一列跟direct同樣數量行的陣列做轉換用
direct=np.arange(1,len(direct_collide)*2+1)
direct=direct.reshape(len(direct_collide),2)
direct=np.append(direct,dircet_append,axis=1)

for i in range(0,len(direct_collide)):
    direct[i][0]=direct_collide[i][0]
    direct[i][1]=direct_collide[i][1]
    direct[i][2]=1

circle_append=np.zeros((len(circleplot),1))#化成3
#print(circle_append)
circleplot=np.append(circleplot,circle_append,axis=1)
for i in range(0,len(circleplot)):
    circleplot[i][2]=1

flip_append=np.zeros((len(flip_once_pointplot),1))#化成3
print(len(flip_append))
flip=np.arange(1,len(flip_once_pointplot)*2+1)
flip=flip.reshape(len(flip_once_pointplot),2)

flip=np.append(flip,flip_append,axis=1)

for i in range(0,len(flip_once_pointplot)):
    flip[i][0]=flip_once_pointplot[i][0]
    flip[i][1]=flip_once_pointplot[i][1]
    flip[i][2]=1

#print(flip)
#print(flip_once_pointplot)


with open("D:/matlab/hiwin_robotic_arm/intrinsic_matrix33", 'r') as Intrinsic_csv:#csv內參讀取
     Intrinsic_csv = list(csv.reader(Intrinsic_csv, delimiter=","))
with open("D:/matlab/hiwin_robotic_arm/translation_vector13", 'r') as TranslationVector_csv:
     TranslationVector_csv = list(csv.reader(TranslationVector_csv, delimiter=","))
with open("D:/matlab/hiwin_robotic_arm/extrinsic_matrix33", 'r') as RotationMatrix_csv:
     RotationMatrix_csv = list(csv.reader(RotationMatrix_csv, delimiter=","))
Intrinsic = np.array(Intrinsic_csv)
RotationMatrix = np.array(RotationMatrix_csv)
TranslationVector= np.array(TranslationVector_csv)
ExtrinsicMatrix=np.zeros((3,3))
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
'''
theta=2.725*math.pi/180
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
Intrinsic=Intrinsic.astype(float)
matrix_H=np.matmul(Intrinsic,ExtrinsicMatrix)#內外參相乘
matrix_H_inv=np.linalg.inv(matrix_H)#返乘矩陣
def s(x,y):
    s=matrix_H[2][0]*x+matrix_H[2][1]*y+matrix_H[2][2] #尺度因子 為Zc分之1
    return s


#乘上內外參
final_piccoor=(np.matmul(matrix_H,final_coor.T)).T
direct_piccoor=(np.matmul(matrix_H,direct.T)).T
circle_piccoor=(np.matmul(matrix_H,circleplot.T)).T
flip_piccoor=(np.matmul(matrix_H,flip.T)).T
#print(flip_piccoor)

#最終打擊點轉圖像
final_piccoor[0]=final_piccoor[0]/s(final_coor.T[0],final_coor.T[1])
final_piccoor[1]=final_piccoor[1]/s(final_coor.T[0],final_coor.T[1])
final_piccoor[2]=final_piccoor[2]/s(final_coor.T[0],final_coor.T[1])
final_piccoor=final_piccoor
final_piccoor=final_piccoor.astype(int)
print("here")
print(final_piccoor)
cv2.circle(output, (final_piccoor[0],final_piccoor[1] ), 10 ,(200,200,125),2)
cv2.circle(output, (final_piccoor[0],final_piccoor[1] ), 10 ,(0,0,0),2)
#print("最終")
#print(final_piccoor)

for i in range(0,len(direct_piccoor)):
    direct_piccoor[i][0]=direct_piccoor[i][0]/s(direct.T[0][i],direct.T[1][i])
    direct_piccoor[i][1]=direct_piccoor[i][1]/s(direct.T[0][i],direct.T[1][i])
    direct_piccoor[i][2]=direct_piccoor[i][2]/s(direct.T[0][i],direct.T[1][i])

direct_piccoor=direct_piccoor
direct_piccoor=direct_piccoor.astype(int)

for i in range(0,len(circle_piccoor)):#一一將其標準化 此為洞口與母球等等
    circle_piccoor[i][0]=circle_piccoor[i][0]/s(circleplot.T[0][i],circleplot.T[1][i])
    circle_piccoor[i][1]=circle_piccoor[i][1]/s(circleplot.T[0][i],circleplot.T[1][i])
    circle_piccoor[i][2]=circle_piccoor[i][2]/s(circleplot.T[0][i],circleplot.T[1][i])

#print(circle_piccoor)
circle_piccoor=circle_piccoor
circle_piccoor=circle_piccoor.astype(int)


for i in range(0,len(flip_piccoor)):#一一將其標準化 撞擊點
    flip_piccoor[i][0]=flip_piccoor[i][0]/s(flip.T[0][i],flip.T[1][i])
    flip_piccoor[i][1]=flip_piccoor[i][1]/s(flip.T[0][i],flip.T[1][i])
    flip_piccoor[i][2]=flip_piccoor[i][2]/s(flip.T[0][i],flip.T[1][i])

#print(flip_piccoor)
flip_piccoor=flip_piccoor
flip_piccoor=flip_piccoor.astype(int)

#print(circle_piccoor)
#print(flip_piccoor)
output = cv2.resize(original,(0,0),fx=1,fy=1)
for i in range(0,len(direct)):#畫直接撞擊
    if (abs(direct_collide[i][6])<2000):
        cv2.line(output, (circle_piccoor[6][0],circle_piccoor[6][1] ),(direct_piccoor[i][0],direct_piccoor[i][1]),(50,100,125),3)
        print(i)

for i in range(0,8):#畫出洞口及子球
   cv2.circle(output, (circle_piccoor[i][0],circle_piccoor[i][1] ), 10 ,(255,0,0),2)

#cv2.circle(output, (circle_piccoor[6][0],circle_piccoor[6][1] ), 10 ,(255,255,255),2)#母球
print(circle_piccoor[6][0],circle_piccoor[6][1] )

for i in range(8,len(circle_piccoor)):#劃出撞擊點
    cv2.circle(output, (circle_piccoor[i][0],circle_piccoor[i][1] ), 10 ,(50*(i-8),255-50*(i-8),50*(i-8)),2)
    #print(i)

#畫邊緣
cv2.line(output, (circle_piccoor[0][0],circle_piccoor[0][1] ),(circle_piccoor[1][0],circle_piccoor[1][1]),(0,0,0),5)
cv2.line(output, (circle_piccoor[0][0],circle_piccoor[0][1] ),(circle_piccoor[4][0],circle_piccoor[4][1]),(0,0,0),5)
cv2.line(output, (circle_piccoor[1][0],circle_piccoor[1][1] ),(circle_piccoor[5][0],circle_piccoor[5][1]),(0,0,0),5)
cv2.line(output, (circle_piccoor[4][0],circle_piccoor[4][1] ),(circle_piccoor[5][0],circle_piccoor[5][1]),(0,0,0),5)

classify = 7

for i in range(0,len(flip_once_pointplot)):#劃出彈射軌跡
    if (flip_once_pointplot[i][6]==0):
        cv2.circle(output, (flip_piccoor[i][0],flip_piccoor[i][1] ), 2 ,(200,200,125),2)
        cv2.line(output, (circle_piccoor[6][0],circle_piccoor[6][1] ),(flip_piccoor[i][0],flip_piccoor[i][1]),(200,200,125),1)
        cv2.line(output, (flip_piccoor[i][0],flip_piccoor[i][1] ),(circle_piccoor[classify][0],circle_piccoor[classify][1]),(200,200,125),1)
    #print(i)
    #print("餘式")
    #print(i%4)
    if i%4==0:#每四次加一
        classify+=1
        print(classify)


print("no problem")

cv2.imshow('circle out',output)
cv2.waitKey(0)
cv2.destroyAllWindows()






