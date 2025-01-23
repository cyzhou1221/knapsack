# Parallel 0-1 Knapsack Problem Solver
Try to solve 0-1 knapsack problem parallelly by Branch-and-Bound.

## 输入文件格式

- 首行: `物品个数(n),重量限制(c)`
- 其余行: `价值(profit),重量(weight)`
- 文件格式为 `.csv` 文件
比如下面的 $0$-$1$ 背包问题:
$$
\begin{equation}
	\begin{aligned}
		Z = \max 10 x_{1} + 7 x_{2} + 25 x_{3} &+ 24 x_{4} \\
		2 x_{1} + 1 x_{2} + 6 x_{3} &+ 5 x_{4}  \leq 7 \\
		&x \in\{0,1\}^{4} .
	\end{aligned}	
\end{equation}
$$

对应的输入文件为: 
```
	4,7
	10,2
	7,1
	25,6
	24,5
```


## 正确性验证

建立正确性验证算例集 `ctestlib`

**测试算例来源:** 2005 年 Pisinger 发表了一篇关于背包问题求解的难易程度的文章[2],
其中提到, 若物品价值 $p_j$ 与重量 $w_j$ 的相关性较小, 则对应的背包问题是容易求解的. 另一方面,若二者相关性较强, 则对应的背包问题是比较难求解的, 主要有以下两个原因:

1. 在这种情况下, 线性规划松弛的最优值与原问题最优值相差较大, 导致问题是*病态的(ill-conditioned)*.
2. 在这种情况下, 解线性规划松弛时按照价值密度排序基本上对应于按照重量排序, 这导致很难以等式形式满足重量约束.
为此, 我们选择物品价值 $p_j$ 与重量 $w_j$ 相关性较小的例子用于正确性测试: 
1. 选择小系数(small coefficients)类别中的 `knapPI_1_50_1000.csv` 作为测试算例源文件.
2. 选取其中的第 $1,11,21,\dots,91$ 共 $10$ 个例子作为正确性测试算例.
对应的算例(及其最优解)可以在 Pisinger 的[个人网站](http://hjemmesider.diku.dk/~pisinger/codes.html)进行下载.


## 并行测试算例

算例来源同正确性验证算例集, 我们选取物品价值 $p_j$ 与重量 $w_j$ 相关性较强(strongly correlated)的例子用于并行测试(这类例子的串行求解难度较大, 尝试用并行算法求解, 期望能够得到好的效果): 

选择小系数(small coefficients)类别中的 `knapPI_3_200_1000.csv` 中的前 $50$ 个例子作为测试算例.


## Reference
[1] https://github.com/MGomez99/Best-First-Search-BB

[2] Pisinger D. Where are the hard knapsack problems? [J]. Computers & Operations Research, 2005,
32(9):2271–2284.