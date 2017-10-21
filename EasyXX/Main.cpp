#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <stdio.h>

#define TIME 120
#define	CMD_LEFT  1
#define	CMD_RIGHT 2
#define	CMD_SPACE 4
#define Ball_R 8		 //球半径
#define Rank_num 5		 //排行前五

int box_x = 10;
int box_y = 320;

// 定义球的结构体
typedef struct Ball
{
	int x;
	int y;
	int v;
	int kind; //馅饼1 2 3 or 炸弹0
	struct Ball*next;
}Ball;

typedef struct
{
	int x, y, width, height;
} Rect; //矩形区域

struct HighScore //计分
{
	char name[30];
	int score;
};

int mouseX = 0, mouseY = 0, mouseState = 0, GameState = 0, isPause = 0, EndGame = 0, isDead = 0;
char username[30] = { 0 };
time_t firstgame, firstpause;
IMAGE role, background, rock, stone, cake, bomb, para;

bool judgeButton(Rect * rect, int x, int y);
void drawButton(Rect * rect, char *str, int model);
int GetCommand();
int UpdateMouse();

void CreateBall(Ball*Head, int ball_v, int rate);						//生成球
void DeleteOneBall(Ball *p, Ball *s);									//删s节点
Ball*MoveOneBall(Ball*s);												//移动一个球
Ball* MoveBall(Ball*Head);												//移动球
Ball* HideOneBall(Ball*s);												//隐藏
void Score(Ball*s, int *pix1, int*pix2, int*score);		//计分
bool Hit(Ball*s, int *pix1, int*pix2, int *score);			//判断是否碰撞
Ball* MoveBowl(Ball*Head, int *pix1, int*pix2, int*score);	//移动碗 

int Save(int*score);						//记录每次得分
void High(int*score, int outx, int outy);	//显示当前用户最高分
void RankHigh(int*score);					//排行榜排序
void RankHighDisplay();						//排行榜的显示

//判断是否在按钮范围内
bool judgeButton(Rect * rect, int x, int y) {
	if (x >= rect->x && x <= rect->x + rect->width && y >= rect->y && y <= rect->y + rect->height)
		return true;
	return false;
}

void drawButton(Rect * rect, char *str, int model = 0) {

	if (judgeButton(rect, mouseX, mouseY)) {
		setlinecolor(RGB(251, 124, 133));
		if (mouseState == 1) {
			setfillcolor(RGB(255, 218, 196));
		}
		else {
			setfillcolor(RGB(255, 238, 196));
		}
	}
	else {
		setlinecolor(WHITE);
		setfillcolor(RGB(220, 240, 255));
	}
	fillroundrect(rect->x, rect->y, rect->x + rect->width, rect->y + rect->height, 10, 10);
	setbkmode(TRANSPARENT);
	RECT r1 = { rect->x,rect->y,rect->x + rect->width,rect->y + rect->height };
	settextstyle(13, 0, _T(""));
	settextcolor(BLACK);
	drawtext(str, &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

int GetCommand()//键盘按键
{
	int c = 0;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)  c |= CMD_LEFT;//GetAsyncKeyState，判断函数调用时指定虚拟键的状态
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) c |= CMD_RIGHT;
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) c |= CMD_SPACE;
	return c;
}

int UpdateMouse()
{
	MOUSEMSG MouseMsg;
	if (MouseHit() == false) {
		return 0;
	}
	MouseMsg = GetMouseMsg();
	mouseX = MouseMsg.x;
	mouseY = MouseMsg.y;
	if (MouseMsg.uMsg == WM_LBUTTONDOWN) {
		mouseState = 1;
	}
	else {
		mouseState = 0;
	}
	return mouseState;
}

//时间，间隔时间
int Time(time_t first)//记时
{
	int t;
	time_t second;
	second = time(NULL);                       //获取时间
	t = (int)difftime(second, first);          //计算连个间隔的时间差
	return t;
}
bool Interval(double T, clock_t *t1)//间隔固定时间毫秒级的10倍
{
	bool ifT = false;
	clock_t t2;
	t2 = clock();
	if ((t2 - *t1) / 10 == T)
	{
		ifT = true;
		*t1 = t2;
	}
	return ifT;
}

void NumDisplay(int x, int y, int *num)//数字显示
{
	TCHAR n[50];
	_stprintf(n, _T("%d"), *num);              //数字转换字符
	outtextxy(x, y, n);
}
//数字显示，动态显示
void DynamicDisplay(int x1, int x2, int *num)//动态显示（400，420区域）
{
	settextcolor(BLACK);
	clearrectangle(x1, 400, x2, 420);           //清除
	NumDisplay(x1, 400, num);
}

void DrawMenu()
{
	Rect btnStart = { 260,200,120,35 }, btnRank = { 260,240,120,35 }, btnQuit = { 260,280,120,35 };
	BeginBatchDraw();
	drawButton(&btnStart, _T("开始游戏"));
	drawButton(&btnRank, _T("排行榜"));
	drawButton(&btnQuit, _T("退出游戏"));
	if (judgeButton(&btnStart, mouseX, mouseY) && mouseState == 1) {
		if (username[0] == 0)
			InputBox(username, 10, _T("请输入用户名"));
		GameState = 2;
		cleardevice();
	}
	if (judgeButton(&btnRank, mouseX, mouseY) && mouseState == 1) {
		GameState = 1;
		cleardevice();
	}
	if (judgeButton(&btnQuit, mouseX, mouseY) && mouseState == 1) {
		cleardevice();
		exit(0);
	}
	EndBatchDraw();
}

void DrawRank()
{
	Rect btnBack = { 260,320,120,35 };
	BeginBatchDraw();
	setbkcolor(RGB(251, 254, 233));
	setbkmode(TRANSPARENT);//透明，没加此函数之前文字底下有白色底！
	cleardevice();     //用背景色填充屏幕 

	LOGFONT f;                            //标题  
	gettextstyle(&f);                     // 获取当前字体设置？
	f.lfHeight = 53;
	_tcscpy_s(f.lfFaceName, _T("微软雅黑"));
	f.lfQuality = ANTIALIASED_QUALITY;    // 输出效果为抗锯齿！！
	settextstyle(&f);
	settextcolor(RGB(90, 68, 70));
	outtextxy(260, 120, _T("排行榜"));
	drawButton(&btnBack, _T("返回菜单"));
	RankHighDisplay();
	if (judgeButton(&btnBack, mouseX, mouseY) && mouseState == 1) {
		GameState = 0;
		cleardevice();
	}
	EndBatchDraw();
}

void DrawGame(int*score)
{
	Rect btnStart = { 30,435,90,25 }, btnEnd = { 130,435,90,25 }, btnSave = { 230,435,90,25 }, btnHigh = { 330,435,90,25 }, btnQuit = { 430,435,90,25 };
	BeginBatchDraw();
	setlinecolor(RGB(90, 68, 70));
	line(0, 381, 640, 381);

	if (GameState == 2) {
		drawButton(&btnStart, _T("开始游戏"));
	}
	else if (GameState == 3) {
		drawButton(&btnStart, _T("暂停游戏"));
	}
	else if (GameState == 4) {
		drawButton(&btnStart, _T("继续游戏"));
	}

	drawButton(&btnEnd, _T("结束游戏"));
	drawButton(&btnSave, _T("保存成绩"));
	drawButton(&btnHigh, _T("显示最高分"));
	drawButton(&btnQuit, _T("退出"));

	settextcolor(BLACK);
	outtextxy(50, 400, _T("用户名："));
	outtextxy(120, 400, username);//显示用户名
	outtextxy(280, 400, _T("本盘得分："));
	outtextxy(400, 400, _T("最高分："));
	outtextxy(510, 400, _T("剩余时间："));

	if (judgeButton(&btnStart, mouseX, mouseY) && mouseState == 1) {
		if (GameState == 2) {
			GameState = 3;
			isPause = 0;
			firstgame = time(NULL);
		}
		else if (GameState == 3) {
			GameState = 4;
			isPause = 1;
		}
		else if (GameState == 4) {
			GameState = 3;
			isPause = 0;
		}
	}
	if (judgeButton(&btnEnd, mouseX, mouseY) && mouseState == 1) {
		GameState = 2;
		isPause = 1;
		EndGame = 1;
		cleardevice();
	}
	if (judgeButton(&btnQuit, mouseX, mouseY) && mouseState == 1) {
		GameState = 0;
		isPause = 1;
		EndGame = 1;
		cleardevice();
		GameState = 0;
	}
	if (judgeButton(&btnHigh, mouseX, mouseY) && mouseState == 1) {
		clearrectangle(470, 400, 500, 420);           //清除
		High(score, 470, 400);
	}
	EndBatchDraw();
	FlushBatchDraw();
}
void GameOverDraw(int*score)//结束界面绘图
{
	BeginBatchDraw();
	putimage(0, 0, &background);
	setbkmode(TRANSPARENT);//透明，没加此函数之前文字底下有白色底！ 

	LOGFONT f;                            //标题  
	gettextstyle(&f);                     // 获取当前字体设置？
	f.lfHeight = 40;
	_tcscpy_s(f.lfFaceName, _T("微软雅黑"));
	f.lfQuality = ANTIALIASED_QUALITY;    // 输出效果为抗锯齿！！
	settextstyle(&f);
	settextcolor(RGB(251, 124, 133));
	NumDisplay(320, 195, score);
	High(score, 320, 245);

	Rect btnReStart = { 260,300,120,35 }, btnQuit = { 260,340,120,35 }, btnSave = { 260,380,120,35 };

	drawButton(&btnReStart, _T("再来一局"));
	drawButton(&btnQuit, _T("退出游戏"));
	drawButton(&btnSave, _T("保存成绩"));
	if (judgeButton(&btnReStart, mouseX, mouseY) && mouseState == 1) {
		GameState = 2;
		cleardevice();
	}
	if (judgeButton(&btnSave, mouseX, mouseY) && mouseState == 1) {
		Save(score);
		RankHigh(score);
	}
	if (judgeButton(&btnQuit, mouseX, mouseY) && mouseState == 1) {
		cleardevice();
		exit(0);
	}
	EndBatchDraw();
}
//小球移动添加碰撞等
void CreateBall(Ball*Head, int ball_v, int rate)//添加球
{
	Ball *s = NULL, *p;
	s = (Ball*)malloc(sizeof(Ball));
	p = Head;

	s->x = rand() % 624 + 8;
	s->y = 0;
	s->v = rand() % 3 + ball_v;//普通2~4，困难3~5
	s->kind = rand() % rate;//普通1:2(0,1,2)，困难1:1
	s->next = NULL;

	while (p->next != NULL)//遍历
		p = p->next;
	p->next = s;
}
void DeleteOneBall(Ball *p, Ball *s)//删s
{
	p->next = s->next;
	free(s);
}
Ball*MoveOneBall(Ball*s)//移动一个球
{
	s->y += s->v;
	if (s->kind == 0)	//弹
	{
		putimage(s->x - 15, s->y - 15, &bomb);
	}
	else if (s->kind == 1)	//伞
	{
		putimage(s->x - 15, s->y - 15, &para);
	}
	else if (s->kind == 2)	//饼
	{
		putimage(s->x - 15, s->y - 15, &cake);
	}
	else if (s->kind == 3)	//小石头
	{
		putimage(s->x - 15, s->y - 15, &stone);
	}
	else if (s->kind == 4)	//大石头
	{
		putimage(s->x - 15, s->y - 15, &rock);
	}
	return s;
}
Ball*MoveBall(Ball*Head)//移动所有球 
{
	Ball *p, *s;
	p = Head;
	s = p->next;
	while (s != NULL)
	{
		HideOneBall(s);
		MoveOneBall(s);
		if (s->y >= 350)
		{
			HideOneBall(s);
			DeleteOneBall(p, s);
			return Head;
		}
		p = p->next;
		s = s->next;
	}
	return Head;
}
Ball* HideOneBall(Ball*s)//隐藏
{
	setlinecolor(RGB(251, 254, 233));		// 擦掉上次显示的旧图形
	setfillcolor(RGB(251, 254, 233));
	fillrectangle(s->x - 15, s->y -15, s->x + 15, s->y + 15);
	return s;
}
Ball* MoveBowl(Ball*Head, int *pix1, int*pix2, int*score)//移动碗
{
	int c;
	Ball *p, *s;
	p = Head;
	s = p->next;

	c = GetCommand();
	FlushBatchDraw();
	setlinecolor(RGB(251, 254, 233));  // 先擦掉上次显示的旧图形
	setfillcolor(RGB(251, 254, 233));
	fillrectangle(*pix1, 350, *pix1 + 60, 380);
	if (c & CMD_LEFT) { *pix1 -= 5; *pix2 -= 5; }
	if (c & CMD_RIGHT) { *pix1 += 5; *pix2 += 5; }

	if (*pix1 < 0) { *pix1 = 0; *pix2 = 60; }
	if (*pix2 > 640) { *pix1 = 580; *pix2 = 640; }

	setlinecolor(BROWN);  // 绘制新的图形
						  //pie(*pix1, 320, *pix2, 380, 3.14, 0);
	putimage(*pix1, 350, &role);
	for (; s != NULL; p = p->next, s = s->next)
		if (Hit(s, pix1, pix2, score))
		{
			DeleteOneBall(p, s);
			return Head;
		}
	return Head;
}
void Score(Ball*s, int *pix1, int*pix2, int*score)//计分
{
	if (s->kind == 0)
	{
		*score -= 100;
		isDead = 1;
		*pix1 = 290, *pix2 = 350;
	}
	else if (s->kind == 1) {
		*score += 0;
	}
	else if (s->kind == 2) {
		*score += 50;
	}
	else if (s->kind == 3) {
		*score -= 10;
	}
	else if (s->kind == 4) {
		*score -= 20;
	}
}
bool Hit(Ball*s, int *pix1, int*pix2, int *score)//判断是否碰撞
{
	bool ishit = false;
	if (s->y >= 320 && s->x< *pix2 && s->x>*pix1)
	{
		ishit = true;
		setlinecolor(RGB(251, 254, 233));  // 先擦掉上次显示的旧图形
		setfillcolor(RGB(251, 254, 233));
		fillrectangle(*pix1, 350, *pix1 + 60, 380);
		fillrectangle(s->x - 15, s->y - 15, s->x + 15, s->y + 15);
		Score(s, pix1, pix2, score);
	}
	return ishit;
}
void Init(int *pix1, int*pix2, int*score)//初始化 
{
	*pix1 = 290;            //碗初始位置
	*pix2 = 350;
	*score = 0;
	isDead = 0;
}

int Suspend(time_t first, int*score)//暂停
{
	Rect btnStart = { 30,435,90,25 };
	while (isPause == 1)
	{
		if (MouseHit()) {
			UpdateMouse();
			DrawGame(score);
		}
	}
	return Time(first);
}

void GameRun(int*score, int gap, int ball_v, int rate)   //游戏运行                 
{
	int pix1, pix2;   //碗初始位置 剩余碗的数量  
	int count1 = 0, count2 = 0, game_time = 0, pause_time = 0;
	time_t firstgame, firstpause;
	clock_t firstinterval_creat, firstinterval_move;

	char c;
	c = GetCommand();
	Ball *H;
	H = (Ball*)malloc(sizeof(Ball)); //头节点
	H->next = NULL;

	Init(&pix1, &pix2, score);     //初始化
	DrawGame(score); //界面绘制
	BeginBatchDraw();
	if (isPause == 0)
	{
		firstgame = time(NULL);//游戏时间的第一次获取
		firstinterval_move = clock();//间隔时间的第一次获取
		firstinterval_creat = clock();//间隔时间的第一次获取
		while (true)
		{
			c = GetCommand();
			FlushBatchDraw();
			if (Interval(gap, &firstinterval_creat))//隔1秒生成球
				CreateBall(H, ball_v, rate);
			if (Interval(2, &firstinterval_move))//20ms移动
			{
				MoveBall(H);
				MoveBowl(H, &pix1, &pix2, score);
			}
			game_time = TIME - Time(firstgame) + pause_time;//计算时间

			DynamicDisplay(580, 630, &game_time);//显示时间  
			DynamicDisplay(350, 380, score);  //显示分数
			if (MouseHit()) {
				UpdateMouse();
				DrawGame(score);
				if (EndGame == 1 || GameState == 0) {
					return;
				}
			}
			if (c & CMD_SPACE || isPause)//暂停
			{
				firstpause = time(NULL);
				pause_time += Suspend(firstpause, score);
				firstinterval_creat = clock();//重新获取间隔时间，否则不能在整间隔时间出现球
				firstinterval_move = clock();
			}
			if (game_time == 0 || isDead == 1)
				GameState = 5;
			if (GameState == 5) {
				cleardevice();
				return;
			}
			if (EndGame == 1) {
				cleardevice();
				EndGame = 0;
				return;
			}

		}
	}
	EndBatchDraw();
}

void RankHighDisplay()//排行榜的显示
{
	FILE * fp;//最高分文件
	struct HighScore s[Rank_num];
	int i;
	fp = fopen("highscore.txt", "r+");
	if (fp != NULL) {
		while (!feof(fp))
			for (i = 0; i < Rank_num; i++)
				fscanf(fp, "%d%s", &s[i].score, s[i].name);
		for (i = 0; i < Rank_num; i++)
		{
			outtextxy(250, 200 + 20 * i, s[i].name);
			NumDisplay(340, 200 + 20 * i, &s[i].score);
		}
		fclose(fp);
	}
}

void High(int*score, int outx, int outy)//显示当前用户最高分
{
	FILE * fp;
	int flag = 0;//是否存过成绩
	struct HighScore s;
	int highscore = *score;
	fp = fopen("score.txt", "r+");
	if (fp != NULL) {
		while (!feof(fp)) {
			fscanf(fp, "%s%d", s.name, &s.score);
			if (strcmp(s.name, username) == 0)
				if (highscore < s.score)
					highscore = s.score;

		}
		NumDisplay(outx, outy, &highscore);
		fclose(fp);
	}
}
int Save(int*score)//记录每次得分
{
	FILE * fp;
	fp = fopen("score.txt", "a+");
	if (fp != NULL) {
		fprintf(fp, "\n%4s%16d", username, *score);
		fclose(fp);
	}
	return 0;
}
void RankHigh(int*score)//排行榜排序
{
	FILE * fp;//记录最高分
	struct HighScore s[Rank_num], temp;
	int i, j, max;
	fp = fopen("highscore.txt", "r+");
	if (fp != NULL) {
		while (!feof(fp))
			for (int i = 0; i < Rank_num; i++)
				fscanf(fp, "%d%s", &s[i].score, s[i].name);
		fclose(fp);
	}

	fp = fopen("highscore.txt", "w+");
	if (fp != NULL) {
		for (i = 0; i < Rank_num; i++)
			if (s[i].score < *score)//新最高分
			{
				s[Rank_num - 1].score = *score;
				strcpy(s[Rank_num - 1].name, username);
				break;
			}
		for (i = 0; i < Rank_num - 1; i++)//排序
		{
			max = i;
			for (j = i + 1; j < Rank_num; j++)
			{
				if (s[j].score > s[max].score)
					max = j;
			}
			if (max != i)
			{
				temp.score = s[i].score;
				s[i].score = s[max].score;
				s[max].score = temp.score;
				strcpy(temp.name, s[i].name);
				strcpy(s[i].name, s[max].name);
				strcpy(s[max].name, temp.name);
			}
		}
		for (int i = 0; i < Rank_num; i++)
			fprintf(fp, "\n%16d%4s", s[i].score, s[i].name);
		fclose(fp);
	}
}
int main() {
	initgraph(640, 480);
	setbkcolor(RGB(251, 254, 233));
	setbkmode(TRANSPARENT);
	cleardevice();     //用背景色填充屏幕 
	int score;
	loadimage(&role, "role.jpg");
	loadimage(&background, _T("gameover.jpg"));
	loadimage(&bomb, _T("bomb.bmp"));
	loadimage(&cake, _T("cake.bmp"));
	loadimage(&rock, _T("rock.bmp"));
	loadimage(&stone, _T("stone.bmp"));
	loadimage(&para, _T("parachute.bmp"));
	while (true) {
		UpdateMouse();
		switch (GameState) {
		case 0:
			DrawMenu();
			break;
		case 1:
			DrawRank();
			break;
		case 2:
			DrawGame(&score);
			break;
		case 3:
			DrawGame(&score);
			GameRun(&score, 70, 2, 5);
			break;
		case 4:
			DrawGame(&score);
			break;
		case 5:
			GameOverDraw(&score);
			break;
		default:
			DrawMenu();
			break;
		}
	}
	return 0;
}