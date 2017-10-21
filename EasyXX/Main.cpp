#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <stdio.h>

#define TIME 120
#define	CMD_LEFT  1
#define	CMD_RIGHT 2
#define	CMD_SPACE 4
#define Ball_R 8		 //��뾶
#define Rank_num 5		 //����ǰ��

int box_x = 10;
int box_y = 320;

// ������Ľṹ��
typedef struct Ball
{
	int x;
	int y;
	int v;
	int kind; //�ڱ�1 2 3 or ը��0
	struct Ball*next;
}Ball;

typedef struct
{
	int x, y, width, height;
} Rect; //��������

struct HighScore //�Ʒ�
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

void CreateBall(Ball*Head, int ball_v, int rate);						//������
void DeleteOneBall(Ball *p, Ball *s);									//ɾs�ڵ�
Ball*MoveOneBall(Ball*s);												//�ƶ�һ����
Ball* MoveBall(Ball*Head);												//�ƶ���
Ball* HideOneBall(Ball*s);												//����
void Score(Ball*s, int *pix1, int*pix2, int*score);		//�Ʒ�
bool Hit(Ball*s, int *pix1, int*pix2, int *score);			//�ж��Ƿ���ײ
Ball* MoveBowl(Ball*Head, int *pix1, int*pix2, int*score);	//�ƶ��� 

int Save(int*score);						//��¼ÿ�ε÷�
void High(int*score, int outx, int outy);	//��ʾ��ǰ�û���߷�
void RankHigh(int*score);					//���а�����
void RankHighDisplay();						//���а����ʾ

//�ж��Ƿ��ڰ�ť��Χ��
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

int GetCommand()//���̰���
{
	int c = 0;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)  c |= CMD_LEFT;//GetAsyncKeyState���жϺ�������ʱָ���������״̬
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

//ʱ�䣬���ʱ��
int Time(time_t first)//��ʱ
{
	int t;
	time_t second;
	second = time(NULL);                       //��ȡʱ��
	t = (int)difftime(second, first);          //�������������ʱ���
	return t;
}
bool Interval(double T, clock_t *t1)//����̶�ʱ����뼶��10��
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

void NumDisplay(int x, int y, int *num)//������ʾ
{
	TCHAR n[50];
	_stprintf(n, _T("%d"), *num);              //����ת���ַ�
	outtextxy(x, y, n);
}
//������ʾ����̬��ʾ
void DynamicDisplay(int x1, int x2, int *num)//��̬��ʾ��400��420����
{
	settextcolor(BLACK);
	clearrectangle(x1, 400, x2, 420);           //���
	NumDisplay(x1, 400, num);
}

void DrawMenu()
{
	Rect btnStart = { 260,200,120,35 }, btnRank = { 260,240,120,35 }, btnQuit = { 260,280,120,35 };
	BeginBatchDraw();
	drawButton(&btnStart, _T("��ʼ��Ϸ"));
	drawButton(&btnRank, _T("���а�"));
	drawButton(&btnQuit, _T("�˳���Ϸ"));
	if (judgeButton(&btnStart, mouseX, mouseY) && mouseState == 1) {
		if (username[0] == 0)
			InputBox(username, 10, _T("�������û���"));
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
	setbkmode(TRANSPARENT);//͸����û�Ӵ˺���֮ǰ���ֵ����а�ɫ�ף�
	cleardevice();     //�ñ���ɫ�����Ļ 

	LOGFONT f;                            //����  
	gettextstyle(&f);                     // ��ȡ��ǰ�������ã�
	f.lfHeight = 53;
	_tcscpy_s(f.lfFaceName, _T("΢���ź�"));
	f.lfQuality = ANTIALIASED_QUALITY;    // ���Ч��Ϊ����ݣ���
	settextstyle(&f);
	settextcolor(RGB(90, 68, 70));
	outtextxy(260, 120, _T("���а�"));
	drawButton(&btnBack, _T("���ز˵�"));
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
		drawButton(&btnStart, _T("��ʼ��Ϸ"));
	}
	else if (GameState == 3) {
		drawButton(&btnStart, _T("��ͣ��Ϸ"));
	}
	else if (GameState == 4) {
		drawButton(&btnStart, _T("������Ϸ"));
	}

	drawButton(&btnEnd, _T("������Ϸ"));
	drawButton(&btnSave, _T("����ɼ�"));
	drawButton(&btnHigh, _T("��ʾ��߷�"));
	drawButton(&btnQuit, _T("�˳�"));

	settextcolor(BLACK);
	outtextxy(50, 400, _T("�û�����"));
	outtextxy(120, 400, username);//��ʾ�û���
	outtextxy(280, 400, _T("���̵÷֣�"));
	outtextxy(400, 400, _T("��߷֣�"));
	outtextxy(510, 400, _T("ʣ��ʱ�䣺"));

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
		clearrectangle(470, 400, 500, 420);           //���
		High(score, 470, 400);
	}
	EndBatchDraw();
	FlushBatchDraw();
}
void GameOverDraw(int*score)//���������ͼ
{
	BeginBatchDraw();
	putimage(0, 0, &background);
	setbkmode(TRANSPARENT);//͸����û�Ӵ˺���֮ǰ���ֵ����а�ɫ�ף� 

	LOGFONT f;                            //����  
	gettextstyle(&f);                     // ��ȡ��ǰ�������ã�
	f.lfHeight = 40;
	_tcscpy_s(f.lfFaceName, _T("΢���ź�"));
	f.lfQuality = ANTIALIASED_QUALITY;    // ���Ч��Ϊ����ݣ���
	settextstyle(&f);
	settextcolor(RGB(251, 124, 133));
	NumDisplay(320, 195, score);
	High(score, 320, 245);

	Rect btnReStart = { 260,300,120,35 }, btnQuit = { 260,340,120,35 }, btnSave = { 260,380,120,35 };

	drawButton(&btnReStart, _T("����һ��"));
	drawButton(&btnQuit, _T("�˳���Ϸ"));
	drawButton(&btnSave, _T("����ɼ�"));
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
//С���ƶ������ײ��
void CreateBall(Ball*Head, int ball_v, int rate)//�����
{
	Ball *s = NULL, *p;
	s = (Ball*)malloc(sizeof(Ball));
	p = Head;

	s->x = rand() % 624 + 8;
	s->y = 0;
	s->v = rand() % 3 + ball_v;//��ͨ2~4������3~5
	s->kind = rand() % rate;//��ͨ1:2(0,1,2)������1:1
	s->next = NULL;

	while (p->next != NULL)//����
		p = p->next;
	p->next = s;
}
void DeleteOneBall(Ball *p, Ball *s)//ɾs
{
	p->next = s->next;
	free(s);
}
Ball*MoveOneBall(Ball*s)//�ƶ�һ����
{
	s->y += s->v;
	if (s->kind == 0)	//��
	{
		putimage(s->x - 15, s->y - 15, &bomb);
	}
	else if (s->kind == 1)	//ɡ
	{
		putimage(s->x - 15, s->y - 15, &para);
	}
	else if (s->kind == 2)	//��
	{
		putimage(s->x - 15, s->y - 15, &cake);
	}
	else if (s->kind == 3)	//Сʯͷ
	{
		putimage(s->x - 15, s->y - 15, &stone);
	}
	else if (s->kind == 4)	//��ʯͷ
	{
		putimage(s->x - 15, s->y - 15, &rock);
	}
	return s;
}
Ball*MoveBall(Ball*Head)//�ƶ������� 
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
Ball* HideOneBall(Ball*s)//����
{
	setlinecolor(RGB(251, 254, 233));		// �����ϴ���ʾ�ľ�ͼ��
	setfillcolor(RGB(251, 254, 233));
	fillrectangle(s->x - 15, s->y -15, s->x + 15, s->y + 15);
	return s;
}
Ball* MoveBowl(Ball*Head, int *pix1, int*pix2, int*score)//�ƶ���
{
	int c;
	Ball *p, *s;
	p = Head;
	s = p->next;

	c = GetCommand();
	FlushBatchDraw();
	setlinecolor(RGB(251, 254, 233));  // �Ȳ����ϴ���ʾ�ľ�ͼ��
	setfillcolor(RGB(251, 254, 233));
	fillrectangle(*pix1, 350, *pix1 + 60, 380);
	if (c & CMD_LEFT) { *pix1 -= 5; *pix2 -= 5; }
	if (c & CMD_RIGHT) { *pix1 += 5; *pix2 += 5; }

	if (*pix1 < 0) { *pix1 = 0; *pix2 = 60; }
	if (*pix2 > 640) { *pix1 = 580; *pix2 = 640; }

	setlinecolor(BROWN);  // �����µ�ͼ��
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
void Score(Ball*s, int *pix1, int*pix2, int*score)//�Ʒ�
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
bool Hit(Ball*s, int *pix1, int*pix2, int *score)//�ж��Ƿ���ײ
{
	bool ishit = false;
	if (s->y >= 320 && s->x< *pix2 && s->x>*pix1)
	{
		ishit = true;
		setlinecolor(RGB(251, 254, 233));  // �Ȳ����ϴ���ʾ�ľ�ͼ��
		setfillcolor(RGB(251, 254, 233));
		fillrectangle(*pix1, 350, *pix1 + 60, 380);
		fillrectangle(s->x - 15, s->y - 15, s->x + 15, s->y + 15);
		Score(s, pix1, pix2, score);
	}
	return ishit;
}
void Init(int *pix1, int*pix2, int*score)//��ʼ�� 
{
	*pix1 = 290;            //���ʼλ��
	*pix2 = 350;
	*score = 0;
	isDead = 0;
}

int Suspend(time_t first, int*score)//��ͣ
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

void GameRun(int*score, int gap, int ball_v, int rate)   //��Ϸ����                 
{
	int pix1, pix2;   //���ʼλ�� ʣ���������  
	int count1 = 0, count2 = 0, game_time = 0, pause_time = 0;
	time_t firstgame, firstpause;
	clock_t firstinterval_creat, firstinterval_move;

	char c;
	c = GetCommand();
	Ball *H;
	H = (Ball*)malloc(sizeof(Ball)); //ͷ�ڵ�
	H->next = NULL;

	Init(&pix1, &pix2, score);     //��ʼ��
	DrawGame(score); //�������
	BeginBatchDraw();
	if (isPause == 0)
	{
		firstgame = time(NULL);//��Ϸʱ��ĵ�һ�λ�ȡ
		firstinterval_move = clock();//���ʱ��ĵ�һ�λ�ȡ
		firstinterval_creat = clock();//���ʱ��ĵ�һ�λ�ȡ
		while (true)
		{
			c = GetCommand();
			FlushBatchDraw();
			if (Interval(gap, &firstinterval_creat))//��1��������
				CreateBall(H, ball_v, rate);
			if (Interval(2, &firstinterval_move))//20ms�ƶ�
			{
				MoveBall(H);
				MoveBowl(H, &pix1, &pix2, score);
			}
			game_time = TIME - Time(firstgame) + pause_time;//����ʱ��

			DynamicDisplay(580, 630, &game_time);//��ʾʱ��  
			DynamicDisplay(350, 380, score);  //��ʾ����
			if (MouseHit()) {
				UpdateMouse();
				DrawGame(score);
				if (EndGame == 1 || GameState == 0) {
					return;
				}
			}
			if (c & CMD_SPACE || isPause)//��ͣ
			{
				firstpause = time(NULL);
				pause_time += Suspend(firstpause, score);
				firstinterval_creat = clock();//���»�ȡ���ʱ�䣬�������������ʱ�������
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

void RankHighDisplay()//���а����ʾ
{
	FILE * fp;//��߷��ļ�
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

void High(int*score, int outx, int outy)//��ʾ��ǰ�û���߷�
{
	FILE * fp;
	int flag = 0;//�Ƿ����ɼ�
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
int Save(int*score)//��¼ÿ�ε÷�
{
	FILE * fp;
	fp = fopen("score.txt", "a+");
	if (fp != NULL) {
		fprintf(fp, "\n%4s%16d", username, *score);
		fclose(fp);
	}
	return 0;
}
void RankHigh(int*score)//���а�����
{
	FILE * fp;//��¼��߷�
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
			if (s[i].score < *score)//����߷�
			{
				s[Rank_num - 1].score = *score;
				strcpy(s[Rank_num - 1].name, username);
				break;
			}
		for (i = 0; i < Rank_num - 1; i++)//����
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
	cleardevice();     //�ñ���ɫ�����Ļ 
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