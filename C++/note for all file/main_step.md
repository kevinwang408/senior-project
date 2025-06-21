summary
讀取CSV檔案，球位置、牆位置?、洞位置 (在project中，將wall設定為一條平行x or y 軸之直線)
使用shotplanner去決定子球至洞口的可行性
如果沒有，使用彈射 flipplanner
決定最佳打擊，以距離為標準
指令機械手臂去打擊

28  連線
35  讀取母球、子球、洞口、牆壁?、子球數量
42  使用selectClearShots from shotplanner去找出可行之球與洞口
48  若是valid_shot並非零，找出最小值
    numeric_limits<T>可以用來查詢類別T的極限，使用max去設定為double之極限
    使用極大值可以用min來找出後續最小值
61  else
    若找不到可行球，執行彈射
63  會得到所有可彈射的球，彈射的code沒有少，但沒考慮到彈射時母球質心與牆面的接處點不一楊
65  若有可行之彈射球
67  找出路進最短
76  沒有可行球，回報錯誤

