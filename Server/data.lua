#For Path Of Exile [amsco �������ܲ���] ��Ʒ���˰�
#amsco's_Script_V37
#Updated Time 2022.2.32
#
#========�汾˵��=============================================
#
#���İ棺����ȼ�1-70��Ĭ����ʾ�󲿷���Ʒ����ǰ�ڵ�4L��Ʒ��ǿ��ʾ��
#
#���ڰ棺����ȼ�71-90�����μ���ʯ��ҩ����ʾ��
#
#���ڰ棺����ȼ�90+�������ڰ���������θ���ͼ�ֵ��Ʒ��
#
#��ˢ�棺����ȼ�92+����ˢ�汾���ں��ڰ���������θ���ͼ�ֵ��Ʒ��������ʾ����װ����ש�䷽���ͼ�ֵͨ����T3�������˿������ֻ�װ���ӵȡ�
#
#========ʹ�÷���=============================================
#�����׹��˿����ڹ��ʷ���������̨��(̨��ԭ���ù���Ӣ��̨�����˹��ʼ�򷱺����ù��ʼ��)
#
#���غ�filter�ļ��� ���ļ���Ҫ����������ļ����ڣ���win7Ϊ��
#C:\Users\����û���\Documents\MyGames\Path of Exile
#Ȼ���¼��Ϸ���ڡ����á�---������(UI)��---����Ʒ�����ļ��б�---����ѡ��[����]��Ʒ����[asmco].filter��������� �����¶�ȡ������ʾ���سɹ���OK��
#��Alt������ʾ���ص�װ����
#
#����ͱ߿���ɫ����Ʒ��ֵ��ϵ ����>��ɫ>��ɫ>��ɫ>��ɫ>��ɫ>��ɫ
#
#�صȼ�ֵ �Ų� ��ɫ 212 145 63
#һ�ȼ�ֵ ���� ��ɫ 175 96 37
#���ȼ�ֵ ��ɫ ��ɫ 210 0 0
#���ȼ�ֵ ��ɫ ��ɫ 255 0 255
#�ĵȼ�ֵ ��ɫ ��ɫ 255 255 119
#��ȼ�ֵ ��ɫ ��ɫ 136 136 255
#���ȼ�ֵ ��ɫ ��ɫ 74 230 58
#�ߵȼ�ֵ ��ɫ ��ɫ 255 255 255
#Ĭ��ͨ����ɫ 170 158 130
#Ĭ�ϼ���ʯ��ɫ 27 162 155
#��һ����Ŀ���ֳ�������50�ֽڣ�������֧��
#
#ϡ�ж������Ч 300 200 100
#ϡ�ж���������С 60 50 40[70 60 50 Ĭ��40 ע�����û��Ӣ��ע�ͣ��������Զ̣��ؼӴ�һ������]
#
#
#========��������������======================================
#
#01���Զ������������CustomAlertSound "�ļ���(����չ��)" �� CustomAlertSound "�����ļ�·��"
#01������CustomAlertSound "C:/Users/YourName/Documents/My Games/Path of Exile/thesearethegem.wav"
#
#                                            �� �� С      ��     ��    ��    ��     ��     ��     ��     ��    ��     ��    ��           Բ      ��ʯ     ����     ����    ��     ����    ʮ��   ����    ���    ����    ���      �����ķ���
#02���Զ������С��ͼͼ�꡿��MinimapIcon ��С(0 1 2) ��ɫ(Red, Green, Blue, Brown, White, Yellow, Cyan, Grey, Orange, Pink, Purple) ��״(Circle, Diamond, Hexagon, Square, Star, Triangle, Cross, Moon, Raindrop, Kite, Pentagon, UpsideDownHouse)
#02������MinimapIcon 0 Blue Circle //ע�ͣ��� ��ɫ ����
#
#                                       ��     ��    ��    ��     ��     ��     ��    ��     ��     ��     ��
#03���Զ�������������PlayEffect ��ɫ(Red, Green, Blue, Brown, White, Yellow, Cyan, Grey, Orange, Pink, Purple)
#03������PlayEffect Red //ע�ͣ���
#
#04������ϵͳĬ�ϵ���������DisableDropSound
#
#05������ָ����׺����HasExplicitMod "��׺"
#05������HasExplicitMod "Tacati's"
#
#06��������񡿣�Identified �Ƿ�(True False)
#06������Identified False //ע�ͣ�δ����
#
#07������ָ���Թ���ħ��Ʒ����
#07������AnyEnchantment True //ע�ͣ��Ƿ����Թ���ħ
#        HasEnchantment "Enchantment Decree of Force" //ע�ͣ�ָ���Թ���ħ
#
#08�������ܽ������Ʒ����
#08������
#        Hide //ע�ͣ�����
#        ShaperItem True //ע�ͣ��ܽ���Ʒ
#        ��
#        Hide //ע�ͣ�����
#        ElderItem True //ע�ͣ�������Ʒ
#
#09�������侳��Ʒ����
#09������
#        Show //ע�ͣ���ʾ
#        SynthesisedItem True //ע�ͣ��ϳ���Ʒ
#
#10��ή���ͼ����
#10������
#       BlightedMap True //ע�ͣ�ή���ͼ
#
#11�����ܽ��/�ͳ���Ӱ�졿
#11������
#        
#       HasInfluence Shaper Elder //ע�ͣ����ܽ����Ӱ��
#
#       HasInfluence == Shaper Elder //ע�ͣ����ܽ�ͳ���Ӱ��
#
#12��3.11�¹���
#
# HasInfluence //ע�ͣ���Ӱ����� �޲���
# EnchantmentPassiveNode //ע�ͣ������鱦��׺���� �츳������
#
#12��3.12�¹��� ����Ʒ��
#
#12 ������Replica True
#
#13��3.12�¹��� ��ʯ��Ʒ���滻��
#
#13 ������AlternateQuality True
#
#14 ��3.12�¹��� ��ʯƷ�����͡�
#
#14 ������
#GemQualityType Superior 
#GemQualityType Divergent 
#GemQualityType Anomalous 
#GemQualityType Phantasmal
#
#15 ��3.13�¹���1 ����ָ����׺�����жϡ�
#
#15 ������Show HasExplicitMod >=2 "of Haast" "of Tzteosh" "of Ephij"
#
#16 ��3.13�¹���2 ֧���Զ�����Ч������
#
#16 ������Show CustomAlertSound "pop.wav" 300
#
#17 ��3.13�¹���3 �����鱦�ϸ����츳������
#
#17 ������Show EnchantmentPassiveNum > 5
#========================================�ٷ�����˵��===================================================
#����
#	Show	�����������������, ��ʾ��Ʒ��ִ�ж���
#	Hide	�����������������, ������Ʒ��ִ�ж���
#
#�����Ƚ�, �·����� [Operator] ����ʾ�����Ƚ�
#	<	С�
#	>	���
#	=	���
#	<=	С춻���
#	>=	��춻���

#����
#	ItemLevel [Operator] <Level>	0-100	��Ʒ�ȼ�
#	DropLevel [Operator] <Level>	0-100	��Ʒ��ʼ����ĵȼ�
#	Quality [Operator] <Quality>	0-20	Ʒ��
#	Rarity [Operator] <Rarity>	Normal Magic Rare Unique	ϡ�ж�: Normal(��ͨ) Magic(ħ��) Rare(ϡ��) Unique(����)
#	Class <Class>	��Ʒ����	����Ʒҳ���������еľ���Class���Խ��ܵĲ���.
#	���ֻ���� ˫��, ������� ˫�ֽ�, ˫�ָ�, ˫�ִ�, ���ǲ���������˫�������� ��.
#	BaseType <Type>	��Ʒ����	����Ʒҳ����ȥ�����ľ��ǻ�������, ���繭������ȥ, "���ƹ�"������Ʒ��������.
#	���ܱ�ʯ�Ĳ���, �������, ����BaseType��"����", Class��"�������ܱ�ʯ".
#	Sockets [Operator] <Sockets>	0-6	�����
#	LinkedSockets [Operator] <Links>	2-6	������
#	SocketGroup <Group>	R:�� G:�� B:�� W:��	�����ڵ���ɫ���, ���õ��� RGB �䷽���
#	Height [Operator] <Value>	1-4	��Ʒ�ĸ߶�
#	Width [Operator] <Value>	1-2	��Ʒ�Ŀ��
#	HasExplicitMod <Value>		ǰ��׺����, ����: [HasExplicitMod "Tyrannical"] (Tyrannical �������˺� 155% �� 169%)
#	StackSize [Operator] <Value>	1-n	�ѵ�����
#	GemLevel [Operator] <Value>	1-21	��ʯ�ȼ�
#	Identified <Boolean>	True or False	�Ƿ��Ѽ���
#	Corrupted <Boolean>	True or False	�Ƿ�����Ⱦ
#	ElderItem <Boolean>	True or False	��ʦ��Ʒ
#	ShaperItem <Boolean>	True or False	�ܽ�����Ʒ
#	ShapedMap <Boolean>	True or False	�ܽ��ߵ�ͼ
#	MapTier [Operator] <Value>	1-17	��ͼ�׼�
#	FracturedItem <Boolean>	True or False	������Ʒ
#	SynthesisedItem <Boolean>	True or False	׷����Ʒ
#	AnyEnchantment <Boolean>	True or False	�Թ���ħ��׺, Ϳ��, �����鱦
#	HasEnchantment <Value>	��׺����	�Թ���ħ��׺, Ϳ��, �����鱦
#	BlightedMap <Boolean>	True or False	�����ͼ
#	HasInfluence <Value>	Shaper Elder Crusader Redeemer Hunter Warlord	����һ������
#	HasInfluence == <Value>	Shaper Elder Crusader Redeemer Hunter Warlord	������������
#	AreaLevel [Operator] <Value>	1-84	����ȼ�