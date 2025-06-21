先在.h黨裡面設定一些變數
在flip planner中 設定了一個 struct FlipShot 的構造
五格array 母球至牆向量、與牆接觸點、強制目標向量、目標座標、動作標、總距離
建立一個以Flip Shot結構的vector(再研究一下) 其輸入函數為後面五個
但return時是FlipShot的結構

cpp
12  建立一個evaluateFlipShots的函式，
22  對所有wall
23  對所有子球
26  鏡像併計算母球與鏡像距離
36  計算接觸點
43  計算是否阻擋
45  排除自己
46  母球至牆壁
54  牆壁至目標
61  若沒阻擋 使用flip儲存可行子球及其路徑相關資料




