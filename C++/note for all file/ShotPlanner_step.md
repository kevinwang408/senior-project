12  使用布林去做是否阻擋
    x1 y1 起始點 x2 y2 目標點
    obstacles 其他子球
    一個vector<double>可以存一個2D 座標，存入兩個數字即可
34  使用一個selectClearShots去做選擇目標球
    輸入母球座標、洞口座標、其他子球座標
    這邊似乎把目標設定為了直接將母球打進洞口
    要再看看
47  行的emplace_back是將可行之母球與洞口加入到result中
    類似push_back，應該就是stack那種
