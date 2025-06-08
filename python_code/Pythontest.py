
import cv2
import numpy as np
import csv

img = cv2.imread(r"C:\Users\wang8\OneDrive\桌面\機械手臂相機\ball\balltest3_8-1.bmp")
img = cv2.resize(img,(0,0),fx=0.5,fy=0.5)
#print(img.shape)
contrast =50#230
brightness = 130#380
output = img * (contrast/127 + 1) - contrast + brightness

output = np.clip(output, 0, 255)
output = np.uint8(output)
cv2.imshow('samuelbitchmotherfuck',output)
cv2.waitKey(0)
cv2.destroyAllWindows()
gray = cv2.cvtColor(output, cv2.COLOR_BGR2GRAY)
gray = cv2.GaussianBlur(gray,(5, 5),0)
circles = cv2.HoughCircles(gray, cv2.HOUGH_GRADIENT, 1, 20, None, 100, 15, 10, 18)
circles = circles.astype(int)
son = np.array([])
#output = cv2.line(output,(75,170),(725,475),1)
if len(circles) > 0:
    for x, y, r in circles[0]:
        #if(y<443 or y>1177 or x<175 or x>1596):#100 400 100 750
        if(y<170 or y>450 or x<75 or x>725):
            #b = np.where(circles[0] == [x,y,r])
            #c = np.append(c, b)

            continue
        cv2.circle(output, (x, y), r ,(255,0,0),2)
        son = np.append(son,[x,y])
        for h in range(5,10):
            if(output[y+h,x+h][0]>253 and output[y+h,x+h][1]>253 and output[y+h,x+h][2]>253 and output[y-h,x-h][0]>253 and output[y-h,x-h][1]>253 and output[y-h,x-h][2]>253 and output[y+h,x-h][0]>253 and output[y+h,x-h][1]>253 and output[y+h,x-h][2]>253 and output[y-h,x+h][0]>253 and output[y-h,x+h][1]>253 and output[y-h,x+h][2]>253):
                cv2.circle(output, (x, y), r ,(0,0,255),2)
                mother = np.array([x,y])
# c = np.sort(c)
# d = np.delete(circles[0],c,0)
# e = np.where(d == mothers)
# a = np.delete(d, e, 0)  
# son = np.delete(a,[2],1)
# mother = np.delete(mothers,[2])
#print(circles)
son = son.astype(int)
e = np.where(son == mother[0])
son = np.delete(son, e)
f = np.where(son == mother[1])
son = np.delete(son, f)
sons = son.reshape(int(len(son)/2),-1)
#son = son.astype(int)
#e = np.where(son == mother[0])
#sons = np.delete(son,e,0)
#print(e)

#print(sons)
#print(mother)
 
j=0
for i in sons:#計算求個數
    j+=1
childball_num=np.zeros((1,1),int)
childball_num[0]=j

for i in range(0,j):
    sons[i][0]*=2
    sons[i][1]*=2
for i in range(0,2):
    mother[i]*=2


cueball=np.zeros((1,2),int)
cueball[0]=mother#轉二維方便csv讀取

# np.savetxt('pixel_childball.csv', sons, delimiter=',',fmt='%d')
# np.savetxt('pixel_cueball.csv',cueball, delimiter=',',fmt='%d')
# np.savetxt('childball_num.csv', childball_num, delimiter=',',fmt='%d')

#print(mother)


cv2.imshow('samuelbitchmotherfuck',output)
cv2.waitKey(0)
cv2.destroyAllWindows()
print('母球:')
print(mother)
print('子球')
print(sons)
print('----------------------------------------')

with open("C:/Users/samuel901213/Desktop/opencv/IntrinsicMatrix.csv", 'r') as Intrinsic_csv:#csv內參讀取
     Intrinsic_csv = list(csv.reader(Intrinsic_csv, delimiter=","))
with open("C:/Users/samuel901213/Desktop/opencv/TranslationVectors.csv", 'r') as TranslationVector_csv:
     TranslationVector_csv = list(csv.reader(TranslationVector_csv, delimiter=","))
with open("C:/Users/samuel901213/Desktop/opencv/RotationMatrices.csv", 'r') as RotationMatrix_csv:
     RotationMatrix_csv = list(csv.reader(RotationMatrix_csv, delimiter=","))

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
ExtrinsicMatrix[1][2]=TranslationVector[1][0]
ExtrinsicMatrix[2][2]=TranslationVector[2][0]
ExtrinsicMatrix=ExtrinsicMatrix.astype(float)

Intrinsic=Intrinsic.astype(float)

#matrix_H=Intrinsic*ExtrinsicMatrix

matrix_H=np.matmul(Intrinsic,ExtrinsicMatrix)
# print(Intrinsic)
# print(ExtrinsicMatrix)
# print(matrix_H)


matrix_H_inv=np.linalg.inv(matrix_H)
def s(u,v):
    s=matrix_H_inv[2][0]*u+matrix_H_inv[2][1]*v+matrix_H_inv[2][2]
    return s
mother=np.insert(mother,2,1)
mother_T=(np.array([mother])).T

mother_Wcoor=(np.matmul(matrix_H_inv,mother_T)/s(mother[0],mother[1])).T
#print(mother_Wcoor)
#print(sons)
i=0
childball=np.zeros((j,3))
for son in sons:
    son=np.insert(son,2,1)
    childball[i]=son
    i+=1
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


mother_Wcoor=np.delete(mother_Wcoor,2)
mother_Wcoor=np.array([mother_Wcoor])
print(mother_Wcoor)

np.savetxt('son_Wcoor.csv', son_Wcoor_normalize, delimiter=',',fmt='%f')
np.savetxt('mother_Wcoor.csv',mother_Wcoor, delimiter=',',fmt='%f')
np.savetxt('childball_num.csv', childball_num, delimiter=',',fmt='%f')


# uvcoor=s(226,453)*np.matmul(matrix_H,[[0],[0],[1]])
# a=uvcoor[0][0]
# b=uvcoor[1][0]
# img = cv2.imread("C:/Users/samuel901213/Desktop/wow/1225.bmp")
# cv2.circle(img,(int(a),int(b)),1,(0,0,255),3)
# cv2.imshow('samuelbitchmotherfuck',img)
# cv2.waitKey(0)
# cv2.destroyAllWindows()