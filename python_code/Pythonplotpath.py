import cv2
import numpy as np
import csv


original=cv2.imread(r"C:\Users\wang8\OneDrive\桌面\機械手臂相機\ball\20230330w.bmp")
output = cv2.resize(original,(0,0),fx=0.5,fy=0.5)

with open("C:/Users/wang8/source/repos/cflip/cflip/pythonplot.csv", 'r') as pythonplot:#讀取取得資料之座標點
     pythonplot_csv = list(csv.reader(pythonplot, delimiter=","))

     #print(pythonplot_csv)
     pythonplot=np.array(pythonplot_csv)#此為字串
     print(pythonplot)
     pythonplot=pythonplot.astype(float)
     print(pythonplot)

hole=np.arange(1,19)
hole=hole.reshape(6,3)
for i in range(0,6):#將hole取出
    hole[i][0]=pythonplot[i][0]
    hole[i][1]=pythonplot[i][1]
    hole[i][2]=1
print(hole)

cueball=np.zeros(3)
cueball[0]=pythonplot[6][0]
cueball[1]=pythonplot[6][1]
cueball[2]=1

hited_ball=np.zeros(3)
hited_ball[0]=pythonplot[7][0]
hited_ball[1]=pythonplot[7][1]
hited_ball[2]=1

target_hole=np.zeros(3)
target_hole[0]=pythonplot[8][0]
target_hole[1]=pythonplot[8][1]
target_hole[2]=1

goal_coor=np.zeros(3)
goal_coor[0]=pythonplot[9][0]
goal_coor[1]=pythonplot[9][1]
goal_coor[2]=1

flippoint1=np.zeros(3)
flippoint1[0]=pythonplot[10][0]
flippoint1[1]=pythonplot[10][1]
flippoint1[2]=1

flippoint2=np.zeros(3)
flippoint2[0]=pythonplot[11][0]
flippoint2[1]=pythonplot[11][1]
flippoint2[2]=1

if flippoint2[0]==0:
    if flippoint2[1] ==0:
        flippoint2[0]=10000
        flippoint2[1]=10000

print(flippoint2[0])
print(flippoint2[1])



"""
print(cueball)
print(hited_ball)
print(target_hole)
print(goal_coor)
print(flippoint1)
print(flippoint2)
"""

with open("D:/matlab/hiwin_robotic_arm/intrinsic_matrix33", 'r') as Intrinsic_csv:#csv內參讀取
     Intrinsic_csv = list(csv.reader(Intrinsic_csv, delimiter=","))
with open("D:/matlab/hiwin_robotic_arm/translation_vector13", 'r') as TranslationVector_csv:
     TranslationVector_csv = list(csv.reader(TranslationVector_csv, delimiter=","))
with open("D:/matlab/hiwin_robotic_arm/extrinsic_matrix33", 'r') as RotationMatrix_csv:
     RotationMatrix_csv = list(csv.reader(RotationMatrix_csv, delimiter=","))
#print(Intrinsic_csv)
Intrinsic = np.array(Intrinsic_csv)
RotationMatrix = np.array(RotationMatrix_csv)
TranslationVector= np.array(TranslationVector_csv)
ExtrinsicMatrix=np.zeros((3,3))
ExtrinsicMatrix[0][0]=RotationMatrix[0][0]
ExtrinsicMatrix[0][1]=RotationMatrix[0][1]
ExtrinsicMatrix[1][0]=RotationMatrix[1][0]
ExtrinsicMatrix[1][1]=RotationMatrix[1][1]
ExtrinsicMatrix[2][0]=RotationMatrix[2][0]
ExtrinsicMatrix[2][1]=RotationMatrix[2][1]
ExtrinsicMatrix[0][2]=TranslationVector[0][0]
ExtrinsicMatrix[1][2]=TranslationVector[0][1]
ExtrinsicMatrix[2][2]=TranslationVector[0][2]
ExtrinsicMatrix=ExtrinsicMatrix.astype(float)

Intrinsic=Intrinsic.astype(float)
matrix_H=np.matmul(Intrinsic,ExtrinsicMatrix)
matrix_H_inv=np.linalg.inv(matrix_H)#返乘矩陣
#print(matrix_H_inv)
def s(x,y):
    s=matrix_H[2][0]*x+matrix_H[2][1]*y+matrix_H[2][2] #尺度因子 為Zc分之1
    return s

#hole_T=(np.array([hole])).T
#print(hole.T)
#print(matrix_H)

#hole_piccoor=(np.matmul(matrix_H,hole.T)/s(hole.T[0][0],hole.T[1][0])).T#probably wrong

hole_piccoor=(np.matmul(matrix_H,hole.T)).T
print(hole.T)

for i in range(0,6):#一一將其標準化
    hole_piccoor[i][0]=hole_piccoor[i][0]/s(hole.T[0][i],hole.T[1][i])
    hole_piccoor[i][1]=hole_piccoor[i][1]/s(hole.T[0][i],hole.T[1][i])
    hole_piccoor[i][2]=hole_piccoor[i][2]/s(hole.T[0][i],hole.T[1][i])
#print(hole_piccoor)
hole_piccoor=hole_piccoor/2
#print(hole_piccoor)

cueball_T=(np.array([cueball])).T
cueball_piccoor=(np.matmul(matrix_H,cueball_T)/s(cueball_T[0],cueball_T[1])).T
cueball_piccoor=cueball_piccoor/2
cueball_piccoor=cueball_piccoor.astype(int)
print(cueball_piccoor)


hited_ball_T=(np.array([hited_ball])).T
hited_ball_piccoor=(np.matmul(matrix_H,hited_ball_T)/s(hited_ball_T[0],hited_ball_T[1])).T
hited_ball_piccoor=hited_ball_piccoor/2
hited_ball_piccoor=hited_ball_piccoor.astype(int)

#print(hited_ball_piccoor)

target_hole_T=(np.array([target_hole])).T
target_hole_piccoor=(np.matmul(matrix_H,target_hole_T)/s(target_hole_T[0],target_hole_T[1])).T/2
target_hole_piccoor=target_hole_piccoor.astype(int)

#print(target_hole_piccoor)

goal_coor_T=(np.array([goal_coor])).T
goal_coor_piccoor=(np.matmul(matrix_H,goal_coor_T)/s(goal_coor_T[0],goal_coor_T[1])).T/2
goal_coor_piccoor=goal_coor_piccoor.astype(int)

#print(goal_coor_piccoor)

flippoint1_T=(np.array([flippoint1])).T
flippoint1_piccoor=(np.matmul(matrix_H,flippoint1_T)/s(flippoint1_T[0],flippoint1_T[1])).T/2
flippoint1_piccoor=flippoint1_piccoor.astype(int)

#print(flippoint1_piccoor)

flippoint2_T=(np.array([flippoint2])).T
flippoint2_piccoor=(np.matmul(matrix_H,flippoint2_T)/s(flippoint2_T[0],flippoint2_T[1])).T/2
flippoint2_piccoor=flippoint2_piccoor.astype(int)
#print(flippoint2_piccoor)

hole_piccoor=hole_piccoor.astype(int)

cv2.line(output, (hole_piccoor[0][0],hole_piccoor[0][1] ),(hole_piccoor[1][0],hole_piccoor[1][1]),(0,0,0),4)
cv2.line(output, (hole_piccoor[1][0],hole_piccoor[1][1] ),(hole_piccoor[5][0],hole_piccoor[5][1]),(0,0,0),4)
cv2.line(output, (hole_piccoor[5][0],hole_piccoor[5][1] ),(hole_piccoor[4][0],hole_piccoor[4][1]),(0,0,0),4)
cv2.line(output, (hole_piccoor[4][0],hole_piccoor[4][1] ),(hole_piccoor[0][0],hole_piccoor[0][1]),(0,0,0),4)
cv2.arrowedLine(output,(cueball_piccoor[0][0],cueball_piccoor[0][1] ), (flippoint1_piccoor[0][0],flippoint1_piccoor[0][1] ),(150,0,150),2,5,0,0.2)
cv2.arrowedLine(output,(cueball_piccoor[0][0],cueball_piccoor[0][1] ), (flippoint1_piccoor[0][0],flippoint1_piccoor[0][1] ),(150,0,150),2,5,0,0.2)
if abs(flippoint2[0])<=1000:
    cv2.arrowedLine(output,(flippoint1_piccoor[0][0],flippoint1_piccoor[0][1] ), (flippoint2_piccoor[0][0],flippoint2_piccoor[0][1] ),(150,0,150),2,5,0,0.1)
    cv2.arrowedLine(output,(flippoint2_piccoor[0][0],flippoint2_piccoor[0][1] ), (goal_coor_piccoor[0][0],goal_coor_piccoor[0][1] ),(150,0,150),2,5,0,0.1)
else:
    cv2.arrowedLine(output,(flippoint1_piccoor[0][0],flippoint1_piccoor[0][1] ), (goal_coor_piccoor[0][0],goal_coor_piccoor[0][1] ),(150,0,150),2,5,0,0.1)




for i in range(0,6):
    cv2.circle(output, (hole_piccoor[i][0],hole_piccoor[i][1] ), 10 ,(255,0,0),2)
cv2.circle(output, (cueball_piccoor[0][0],cueball_piccoor[0][1] ), 10 ,(0,0,255),2)
cv2.circle(output, (hited_ball_piccoor[0][0],hited_ball_piccoor[0][1] ), 10 ,(0,255,0),2)
cv2.circle(output, (target_hole_piccoor[0][0],target_hole_piccoor[0][1] ), 10 ,(255,255,255),2)
cv2.circle(output, (goal_coor_piccoor[0][0],goal_coor_piccoor[0][1] ), 10 ,(0,0,0),2)
cv2.circle(output, (flippoint1_piccoor[0][0],flippoint1_piccoor[0][1] ), 10 ,(130,130,130),2)
cv2.circle(output, (flippoint2_piccoor[0][0],flippoint2_piccoor[0][1] ), 10 ,(100,0,100),2)

cv2.imshow('circle out',output)
cv2.waitKey(0)
cv2.destroyAllWindows()



