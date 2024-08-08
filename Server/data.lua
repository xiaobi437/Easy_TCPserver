#For Path Of Exile [amsco 国服功能补丁] 物品过滤版
#amsco's_Script_V37
#Updated Time 2022.2.32
#
#========版本说明=============================================
#
#开荒版：建议等级1-70，默认显示大部分物品，对前期的4L物品加强显示。
#
#中期版：建议等级71-90，屏蔽技能石、药剂显示。
#
#后期版：建议等级90+，在中期版基础上屏蔽更多低价值物品。
#
#速刷版：建议等级92+，速刷版本，在后期版基础上屏蔽更多低价值物品。【不显示：黄装，搬砖配方，低价值通货，T3以下命运卡，部分黄装底子等】
#
#========使用方法=============================================
#本配套过滤可用于国际服、国服、台服(台服原版用国际英；台服用了国际简或繁汉化用国际简或繁)
#
#下载好filter文件后， 该文件需要放在下面的文件夹内，以win7为例
#C:\Users\你的用户名\Documents\MyGames\Path of Exile
#然后登录游戏，在【设置】---【界面(UI)】---【物品过滤文件列表】---下拉选择【[国服]物品过滤[asmco].filter】，最后点击 【重新读取】，显示加载成功即OK！
#按Alt可以显示隐藏的装备。
#
#字体和边框颜色与物品价值关系 暗金>红色>粉色>黄色>绿色>蓝色>白色
#
#特等价值 遗产 颜色 212 145 63
#一等价值 暗金 颜色 175 96 37
#二等价值 红色 颜色 210 0 0
#三等价值 粉色 颜色 255 0 255
#四等价值 黄色 颜色 255 255 119
#五等价值 蓝色 颜色 136 136 255
#六等价值 绿色 颜色 74 230 58
#七等价值 白色 颜色 255 255 255
#默认通货颜色 170 158 130
#默认技能石颜色 27 162 155
#另一个项目名字长度限制50字节，超出后不支持
#
#稀有度相关音效 300 200 100
#稀有度相关字体大小 60 50 40[70 60 50 默认40 注意国服没有英文注释，所以略显短，特加大一号字体]
#
#
#========新增过滤设置项======================================
#
#01【自定义过滤音】：CustomAlertSound "文件名(含扩展名)" 或 CustomAlertSound "完整文件路径"
#01举例：CustomAlertSound "C:/Users/YourName/Documents/My Games/Path of Exile/thesearethegem.wav"
#
#                                            大 中 小      红     绿    蓝    褐     白     黄     青     灰    橙     粉    紫           圆      钻石     六角     方块    星     三角    十字   月亮    雨滴    风筝    五角      倒立的房子
#02【自定义掉落小地图图标】：MinimapIcon 大小(0 1 2) 颜色(Red, Green, Blue, Brown, White, Yellow, Cyan, Grey, Orange, Pink, Purple) 形状(Circle, Diamond, Hexagon, Square, Star, Triangle, Cross, Moon, Raindrop, Kite, Pentagon, UpsideDownHouse)
#02举例：MinimapIcon 0 Blue Circle //注释：大 蓝色 三角
#
#                                       红     绿    蓝    褐     白     黄     青    灰     橙     粉     紫
#03【自定义掉落光束】：PlayEffect 颜色(Red, Green, Blue, Brown, White, Yellow, Cyan, Grey, Orange, Pink, Purple)
#03举例：PlayEffect Red //注释：红
#
#04【屏蔽系统默认掉落音】：DisableDropSound
#
#05【过滤指定词缀】：HasExplicitMod "词缀"
#05举例：HasExplicitMod "Tacati's"
#
#06【鉴定与否】：Identified 是否(True False)
#06举例：Identified False //注释：未鉴定
#
#07【过滤指定迷宫附魔物品】：
#07举例：AnyEnchantment True //注释：是否有迷宫附魔
#        HasEnchantment "Enchantment Decree of Force" //注释：指定迷宫附魔
#
#08【过滤塑界或长老物品】：
#08举例：
#        Hide //注释：隐藏
#        ShaperItem True //注释：塑界物品
#        或
#        Hide //注释：隐藏
#        ElderItem True //注释：长老物品
#
#09【过滤忆境物品】：
#09举例：
#        Show //注释：显示
#        SynthesisedItem True //注释：合成物品
#
#10【萎灭地图】：
#10举例：
#       BlightedMap True //注释：萎灭地图
#
#11【受塑界或/和长老影响】
#11举例：
#        
#       HasInfluence Shaper Elder //注释：受塑界或长老影响
#
#       HasInfluence == Shaper Elder //注释：受塑界和长老影响
#
#12【3.11新规则】
#
# HasInfluence //注释：受影响底子 无参数
# EnchantmentPassiveNode //注释：星团珠宝词缀类型 天赋显著点
#
#12【3.12新规则 是赝品】
#
#12 举例：Replica True
#
#13【3.12新规则 宝石是品质替换】
#
#13 举例：AlternateQuality True
#
#14 【3.12新规则 宝石品质类型】
#
#14 举例：
#GemQualityType Superior 
#GemQualityType Divergent 
#GemQualityType Anomalous 
#GemQualityType Phantasmal
#
#15 【3.13新规则1 过滤指定词缀多重判断】
#
#15 举例：Show HasExplicitMod >=2 "of Haast" "of Tzteosh" "of Ephij"
#
#16 【3.13新规则2 支持自定义音效音量】
#
#16 举例：Show CustomAlertSound "pop.wav" 300
#
#17 【3.13新规则3 星团珠宝上附加天赋点数】
#
#17 举例：Show EnchantmentPassiveNum > 5
#========================================官方过滤说明===================================================
#区块
#	Show	如果所有条件都满足, 显示物品并执行动作
#	Hide	如果所有条件都满足, 隐藏物品并执行动作
#
#条件比较, 下方出现 [Operator] 均表示条件比较
#	<	小於
#	>	大於
#	=	等於
#	<=	小於或等於
#	>=	大於或等於

#条件
#	ItemLevel [Operator] <Level>	0-100	物品等级
#	DropLevel [Operator] <Level>	0-100	物品开始掉落的等级
#	Quality [Operator] <Quality>	0-20	品质
#	Rarity [Operator] <Rarity>	Normal Magic Rare Unique	稀有度: Normal(普通) Magic(魔法) Rare(稀有) Unique(传奇)
#	Class <Class>	物品种类	在物品页面上排所列的就是Class可以接受的参数.
#	如果只输入 双手, 将会包含 双手剑, 双手斧, 双手锤, 但是不包含其他双手武器如 弓.
#	BaseType <Type>	物品基底	在物品页面点进去看到的就是基底名称, 例如弓种类点进去, "粗制弓"就是物品基底名称.
#	技能宝石的部分, 例如火球, 他的BaseType是"火球", Class是"主动技能宝石".
#	Sockets [Operator] <Sockets>	0-6	插槽数
#	LinkedSockets [Operator] <Links>	2-6	连线数
#	SocketGroup <Group>	R:红 G:绿 B:蓝 W:白	连线内的颜色组合, 常用的是 RGB 配方组合
#	Height [Operator] <Value>	1-4	物品的高度
#	Width [Operator] <Value>	1-2	物品的宽度
#	HasExplicitMod <Value>		前后缀名称, 例如: [HasExplicitMod "Tyrannical"] (Tyrannical 是物理伤害 155% 到 169%)
#	StackSize [Operator] <Value>	1-n	堆叠数量
#	GemLevel [Operator] <Value>	1-21	宝石等级
#	Identified <Boolean>	True or False	是否已鉴定
#	Corrupted <Boolean>	True or False	是否已污染
#	ElderItem <Boolean>	True or False	尊师物品
#	ShaperItem <Boolean>	True or False	塑界者物品
#	ShapedMap <Boolean>	True or False	塑界者地图
#	MapTier [Operator] <Value>	1-17	地图阶级
#	FracturedItem <Boolean>	True or False	破碎物品
#	SynthesisedItem <Boolean>	True or False	追忆物品
#	AnyEnchantment <Boolean>	True or False	迷宫附魔词缀, 涂油, 星团珠宝
#	HasEnchantment <Value>	词缀名称	迷宫附魔词缀, 涂油, 星团珠宝
#	BlightedMap <Boolean>	True or False	凋落地图
#	HasInfluence <Value>	Shaper Elder Crusader Redeemer Hunter Warlord	其中一个势力
#	HasInfluence == <Value>	Shaper Elder Crusader Redeemer Hunter Warlord	符合两个势力
#	AreaLevel [Operator] <Value>	1-84	区域等级