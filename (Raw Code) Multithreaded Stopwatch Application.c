#include<stdio.h>
#include<time.h>
#include<pthread.h>
#include<windows.h>
#define UP 72
#define DOWN 80
#define ENTER 13

pthread_t inputThread, thread1, thread2, thread3;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int stuckThread1 = 0;

char ch;
int X, Y;
int startStopwatch = 0;
int focusNo = 0;
int isPause = 0;
int stopStopwatch = 0, stopwatches;
int endStopwatchRunning = 0;
int MaxStopwatches = 10;
int mulSkip[11] = {0};
int mainMenuChoice = 0;
int runtimeSingleStopwatchPos = 0;
int loadingLen[] = {7,7,8,7,6,8,7};
#define loadingTot 7
#define loadingRow 5
#define loadingCol 8
char LOADING[loadingTot][loadingRow][loadingCol] =
{
    {
        "L      ",
        "L      ",
        "L      ",
        "L      ",
        "LLLLLL "
    },
    {
        " OOOO  ",
        "O    O ",
        "O    O ",
        "O    O ",
        " OOOO  "
    },
    {
        "   A    ",
        "   A    ",
        "  A A   ",
        " AAAAA  ",
        "A     A "
    },
    {
        "DDDDD  ",
        "D    D ",
        "D    D ",
        "D    D ",
        "DDDDD  "
    },
    {
        "IIIII ",
        "  I   ",
        "  I   ",
        "  I   ",
        "IIIII "
    },
    {
        "NN    N ",
        "N N   N ",
        "N  N  N ",
        "N   N N ",
        "N    NN "
    },
    {
        " GGGG  ",
        "G      ",
        "G   GG ",
        "G    G ",
        " GGGG  "
    }
};
int stopwatchLen[] = {9, 10, 9, 9, 10, 10, 10, 9, 7};
#define stpwchTot 9
#define stpwchRow 7
#define stpwchCol 10
char STOPWATCH[stpwchTot][stpwchRow][stpwchCol] =
{
    {
        " SSSS    ",
        "S    S   ",
        "S        ",
        " SSSS    ",
        "     S   ",
        "S    S   ",
        " SSSS    "
    },
    {
        "TTTTTTT   ",
        "   T      ",
        "   T      ",
        "   T      ",
        "   T      ",
        "   T      ",
        "   T      "
    },
    {
        " OOOO    ",
        "O    O   ",
        "O    O   ",
        "O    O   ",
        "O    O   ",
        "O    O   ",
        " OOOO    "
    },
    {
        "PPPPP    ",
        "P    P   ",
        "P    P   ",
        "PPPPP    ",
        "P        ",
        "P        ",
        "P        "
    },
    {
        "W     W   ",
        "W     W   ",
        "W     W   ",
        "W  W  W   ",
        "W  W  W   ",
        "W W W W   ",
        " W   W    "
    },
    {
        "   A      ",
        " A   A    ",
        "A     A   ",
        "AAAAAAA   ",
        "A     A   ",
        "A     A   ",
        "A     A   "
    },
    {
        "TTTTTTT   ",
        "   T      ",
        "   T      ",
        "   T      ",
        "   T      ",
        "   T      ",
        "   T      "
    },
    {
        " CCCC    ",
        "C    C   ",
        "C        ",
        "C        ",
        "C        ",
        "C    C   ",
        " CCCC    "
    },
    {
        "H     H",
        "H     H",
        "H     H",
        "HHHHHHH",
        "H     H",
        "H     H",
        "H     H"
    }
};
char Stopwatch2d[7][85] =
{
    "  SSSS    TTTTTTT    OOOO    PPPPP    W     W      A      TTTTTTT    CCCC    H     H",
    " S    S      T      O    O   P    P   W     W    A   A       T      C    C   H     H",
    " S           T      O    O   P    P   W     W   A     A      T      C        H     H",
    "  SSSS       T      O    O   PPPPP    W  W  W   AAAAAAA      T      C        HHHHHHH",
    "      S      T      O    O   P        W  W  W   A     A      T      C        H     H",
    " S    S      T      O    O   P        W W W W   A     A      T      C    C   H     H",
    "  SSSS       T       OOOO    P         W   W    A     A      T       CCCC    H     H"
};


//void delay(clock_t msec)
//{
//    msec = msec + clock();
//    while(msec >= clock());
//}

void hidecursor()
{
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void showcursor()
{
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void gotoxy(int x,int y)
{
    COORD position;
    position.X = x;
    position.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
}

struct ConsolePosition
{
    int X;
    int Y;
};

struct ConsolePosition runtimeOptionsConsolePosition, runtimeStopwatchConsolePosition;
int rm, rs;
int h, m, s, ds, ms;
struct Result
{
    int ms;
    int idx;
};

struct Result result[15];

int compare_results(const void *a, const void *b)
{
    struct Result x = *((struct Result*)a);
    struct Result y = *((struct Result*)b);

    if (x.ms > y.ms)
        return -1;
    if (x.ms < y.ms)
        return 1;
    return 0;
}

void toHMS(int mss, int* hh, int* mm, int* ss, int* dd)
{
    *dd = mss % 1000;
    *ss = mss / 1000;
    *mm = *ss / 60;
    *hh = *mm / 60;
    *ss %= 60;
    *mm %= 60;
}

void printWatch(int hh, int mm, int ss, int dd)
{
    printf("%d%d : %d%d : %d%d : %d%d%d", hh/10, hh%10, mm/10, mm%10, ss/10, ss%10, dd/100, (dd%100)/10, dd%10);
}

void printWatchMs(int mss)
{
    int hh, mm, ss, dd;
    toHMS(mss, &hh, &mm, &ss, &dd);
    printWatch(hh, mm, ss, dd);
}

//void printResult(int n)
//{
//    int pos = 3;
//    gotoxy(runtimeStopwatchConsolePosition.X, n+pos++);
//    printf("#Result: (HH : MM : SS : MSS)");
//    int i;
//    for(i = 0; i < n; i++)
//    {
//        if(i == 9)
//            gotoxy(runtimeStopwatchConsolePosition.X, n+pos++);
//        else
//            gotoxy(runtimeStopwatchConsolePosition.X+1, n+pos++);
//        if(i == 0)
//            printf("1st");
//        else if(i == 1)
//            printf("2nd");
//        else if(i == 2)
//            printf("3rd");
//        else
//            printf("%dth", i+1);
//        printf("-> ");
//        printf("%d ", result[i].idx);
//        printf("(");
//        printWatchMs(result[i].ms);
//        printf(")");
//    }
//}

void calcTime(int a)
{
    ms = clock() - a;
    ds = ms % 1000;
    rs = ms / 1000;
    rm = rs / 60;
    h = rm / 60;
    s = rs % 60;
    m = rm % 60;
}

void SingleOfMultipleStopwatches(int pos)
{
    pthread_mutex_lock(&mutex);
    gotoxy(runtimeStopwatchConsolePosition.X, pos+1);
    if(mainMenuChoice)
        printf("%s%d. ", (pos < 10 ? " " : ""), pos);
    printWatch(h, m, s, ds);
    if(mainMenuChoice)
        gotoxy(runtimeStopwatchConsolePosition.X+22, pos+1);
    else
        gotoxy(runtimeStopwatchConsolePosition.X+18, pos+1);
    if(ds < 334)
        printf(" (/)");
    else if(ds < 667)
        printf(" (-)");
    else if(ds < 1000)
        printf(" (\\)");
    pthread_mutex_unlock(&mutex);
}

void *printFocusPointsError()
{
    pthread_testcancel();
    pthread_mutex_lock(&mutex);
    gotoxy(runtimeStopwatchConsolePosition.X-15,
           runtimeStopwatchConsolePosition.Y+runtimeSingleStopwatchPos+2);
    printf("Warning: Maximum number of focus points has reached!");
    pthread_mutex_unlock(&mutex);
    delay(1500);
    pthread_mutex_lock(&mutex);
    gotoxy(runtimeStopwatchConsolePosition.X-15,
           runtimeStopwatchConsolePosition.Y+runtimeSingleStopwatchPos+2);
    printf("                                                     ");
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void printFocusPoints()
{
    pthread_mutex_lock(&mutex);
    if(focusNo == 1)
    {
        gotoxy(runtimeStopwatchConsolePosition.X,
               runtimeStopwatchConsolePosition.Y+runtimeSingleStopwatchPos++);
        printf("No. HH : MM : SS : MSS");
    }
    gotoxy(runtimeStopwatchConsolePosition.X,
           runtimeStopwatchConsolePosition.Y+runtimeSingleStopwatchPos++);
    printf("%s%d. ", (focusNo < 10 ? " " : ""),focusNo);
    printWatchMs(ms);
    pthread_mutex_unlock(&mutex);
}

int liveResultY;
int runtimeOptionY;
int liveResultAsc = 1;

void printLiveResultPos(int n)
{
    char str[][3] = {"st", "nd", "rd", "th"};
    printf("%d%s-> ", n, (n < 4 ? str[n-1] : str[3]));
}

void *printLiveResult()
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    liveResultY = 3;
    pthread_testcancel();
    pthread_mutex_lock(&mutex);
    gotoxy(runtimeStopwatchConsolePosition.X,
           runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY++);
    printf("#Live result(%s):  ", (liveResultAsc ? "ascending" : "descending"));
    pthread_mutex_unlock(&mutex);
    pthread_testcancel();
    pthread_mutex_lock(&mutex);
    gotoxy(runtimeStopwatchConsolePosition.X+1,
           runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY++);
    printf("Pos. No.(HH : MM : SS : MSS)");
    pthread_mutex_unlock(&mutex);
    int pos = 1;
    for(int i = 0; i < stopwatches; i++)
    {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        int j = i;
        if(!liveResultAsc)
            j = stopwatches - i - 1;
        if((liveResultAsc && j > 0 && result[j].ms != result[j-1].ms) ||
                (!liveResultAsc && j < stopwatches - 1 && result[j].ms != result[j+1].ms))
            ++pos;
        gotoxy(runtimeStopwatchConsolePosition.X,
               runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY+i);
        if(pos != 10)
            putchar(' ');
        printLiveResultPos(pos);
        gotoxy(runtimeStopwatchConsolePosition.X+7,
               runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY+j);
        if(result[i].idx != -1)
        {
            printf("%d (", result[i].idx);
            printWatchMs(result[i].ms);
            putchar(')');
        }
        else
            printf("                                      ");
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void cancelThread2(void *arg)
{
    pthread_cancel(thread2);
}

void runStopwatches(int n)
{
    pthread_cleanup_push(cancelThread2, NULL);
    int a = clock(), i, pauseStart = 0, totPauseTime = 0, liveIdx = 0;
    int liveResultPos[n];
    int currResultMs[n];
    isPause = 0;
    for(i = 0; i < n; i++)
        mulSkip[i] = 0, result[i].idx = -1, result[i].ms = i;
    while(1)
    {
        int run = 0;
        for(i = 0; i < n; i++)
        {
            pthread_testcancel();
            if(!i && !isPause)
                calcTime(a + totPauseTime);
            if(!mulSkip[i])
            {
                run = 1;
                if(!isPause)
                    SingleOfMultipleStopwatches(i);
                if(mainMenuChoice)
                    currResultMs[i] = ms;
                else if(!mainMenuChoice && ch == ENTER)
                {
                    ch = 0;
                    if(focusNo < 10)
                    {
                        focusNo++;
                        printFocusPoints();
                    }
                    else
                        pthread_create(&thread2, NULL, printFocusPointsError, NULL);
                }
            }
            else
            {
                if(mulSkip[i] == 1)
                {
                    mulSkip[i]++;
                    result[liveIdx].idx = i;
                    result[liveIdx].ms = currResultMs[i];
                    liveResultPos[i] = liveIdx;
                    liveIdx++;
                }
                pthread_mutex_lock(&mutex);
                gotoxy(runtimeStopwatchConsolePosition.X+22, i+1);
                printf("     ");
                pthread_mutex_unlock(&mutex);
            }
        }
        if(mainMenuChoice && ch == ' ')
        {
            ch = 0;
            for(i = 0; i < n; i++)
                mulSkip[i]++;
        }
        else if(!mainMenuChoice && ch == ' ')
        {
            ch = 0;
            if(isPause)
                totPauseTime += clock() - pauseStart;
            else
                pauseStart = clock();
            isPause = !isPause;
        }
        if(mainMenuChoice)
        {
            pthread_join(thread2, NULL);
            pthread_create(&thread2, NULL, printLiveResult, NULL);
        }
        if(!run)
            break;
    }
//    if(mainMenuChoice)
//    {
//        qsort(result, n, sizeof(struct Result), compare_results);
//        printResult(n);
//    }
    endStopwatchRunning = 1;
    pthread_cleanup_pop(1);
}

//void *printLiveResultStructure()
//{
//    liveResultY = 3;
//    pthread_mutex_lock(&mutex);
//    gotoxy(runtimeStopwatchConsolePosition.X,
//           runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY++);
//    printf("#Live result(ascending): ");
//    gotoxy(runtimeStopwatchConsolePosition.X+1,
//           runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY++);
//    printf("Pos. No.(HH : MM : SS : MSS)");
//    pthread_mutex_unlock(&mutex);
//    for(int i = 0; i < stopwatches; i++)
//    {
//        pthread_mutex_lock(&mutex);
//        if(i == 9)
//            gotoxy(runtimeStopwatchConsolePosition.X,
//                   runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY+i);
//        else
//            gotoxy(runtimeStopwatchConsolePosition.X+1,
//                   runtimeStopwatchConsolePosition.Y+stopwatches+liveResultY+i);
//        if(i == 0)
//            printf("1st");
//        else if(i == 1)
//            printf("2nd");
//        else if(i == 2)
//            printf("3rd");
//        else
//            printf("%dth", i+1);
//        printf("-> ");
//        pthread_mutex_unlock(&mutex);
//    }
//    pthread_exit(NULL);
//}

void *StartPageOfStopwatches(void *stw)
{
    int stopwatches = *(int*)stw;
    system("cls");
    runtimeOptionsConsolePosition.X = 0;
    runtimeOptionsConsolePosition.Y = 0;
    runtimeStopwatchConsolePosition.X = 70;
    runtimeStopwatchConsolePosition.Y = 0;
    runtimeSingleStopwatchPos = 0;
    runtimeOptionY = 0;
    if(mainMenuChoice)
    {
        gotoxy(runtimeOptionsConsolePosition.X,
               runtimeOptionsConsolePosition.Y+runtimeOptionY++);
        printf("#Winner->least time/ascending(Default)");
        runtimeOptionY++;
    }
    gotoxy(runtimeOptionsConsolePosition.X,
           runtimeOptionsConsolePosition.Y+runtimeOptionY++);
    printf("#Options:");
    if(mainMenuChoice)
    {
        gotoxy(runtimeOptionsConsolePosition.X,
               runtimeOptionsConsolePosition.Y+runtimeOptionY++);
        printf("-Press 0~9 to stop corresponding watch.");
        gotoxy(runtimeOptionsConsolePosition.X,
               runtimeOptionsConsolePosition.Y+runtimeOptionY++);
        printf("-Press space to stop all.");
        gotoxy(runtimeOptionsConsolePosition.X,
               runtimeOptionsConsolePosition.Y+runtimeOptionY++);
        printf("-Press o to change the result order.");
        gotoxy(runtimeStopwatchConsolePosition.X, runtimeStopwatchConsolePosition.Y);
        printf("No. HH : MM : SS : MSS");
        for(int i = 0; i < stopwatches; i++)
        {
            gotoxy(runtimeStopwatchConsolePosition.X, runtimeStopwatchConsolePosition.Y+i+1);
            printf(" %d. ", i);
            printf("00 : 00 : 00 : 000");
        }
    }
    else
    {
        gotoxy(runtimeOptionsConsolePosition.X,
               runtimeOptionsConsolePosition.Y+runtimeOptionY++);
        printf("-Press Space to pause/resume.");
        gotoxy(runtimeOptionsConsolePosition.X,
               runtimeOptionsConsolePosition.Y+runtimeOptionY++);
        printf("-Press Enter to focus any point(upto 10).");
        gotoxy(runtimeStopwatchConsolePosition.X, runtimeStopwatchConsolePosition.Y+runtimeSingleStopwatchPos++);
        printf("HH : MM : SS : MSS");
        gotoxy(runtimeStopwatchConsolePosition.X, runtimeStopwatchConsolePosition.Y+runtimeSingleStopwatchPos++);
        printf("00 : 00 : 00 : 000");
        runtimeSingleStopwatchPos++;
        runtimeSingleStopwatchPos++;
        gotoxy(runtimeStopwatchConsolePosition.X, runtimeStopwatchConsolePosition.Y+runtimeSingleStopwatchPos++);
        printf("#Focus points(upto 10):");
    }
    gotoxy(runtimeOptionsConsolePosition.X,
           runtimeOptionsConsolePosition.Y+runtimeOptionY++);
    printf("-Press r to reset.");
    gotoxy(runtimeOptionsConsolePosition.X,
           runtimeOptionsConsolePosition.Y+runtimeOptionY++);
    printf("-Press m for main menu.");
//    gotoxy(runtimeOptionsConsolePosition.X,
//           runtimeOptionsConsolePosition.Y+runtimeOptionY++);
//    printf("-Press e to exit.");

    int eraseLine = strlen("Press Space to start");
    while(1)
    {
        pthread_testcancel();
        if(startStopwatch)
            break;
        gotoxy(45,14);
        printf("press Space to start");
        delay(300);
        gotoxy(45,14);
        printf("                    ");
//        int i;
//        for(i = 0; i < eraseLine; i++)
//            putchar(' ');
        delay(100);
    }
//    if(mainMenuChoice)
//        pthread_create(&thread2, NULL, printLiveResultStructure, NULL);
    runStopwatches(stopwatches);
    pthread_exit(NULL);
}

struct ConsolePosition mainMenuOptionPosition;
int mainMenuChoiceDone = 0;

void *inputFunction()
{
    while(1)
    {
        ch = _getch();
    }
}

void *showWrongInputError()
{
    pthread_mutex_lock(&mutex);
//    hidecursor();
    gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y+2);
    printf("Error: Please enter between 2~10.");
//    showcursor();
    pthread_mutex_unlock(&mutex);
    delay(1500);
    pthread_testcancel();
    pthread_mutex_lock(&mutex);
//    hidecursor();
    gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y+2);
    printf("                                  ");
//    showcursor();
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int startDecoPrint;

int scanInt(int len)
{
    char input[20];
    int num = 0;
    int i = 0;
    int printCursor = 0;

    while (1)
    {
        printCursor = !printCursor;
        pthread_mutex_lock(&mutex);
        gotoxy(mainMenuOptionPosition.X+len+i, mainMenuOptionPosition.Y);
        if(printCursor)
            printf("_");
        else
            printf(" ");
        pthread_mutex_unlock(&mutex);
        delay(100);
        if (ch == ENTER)
        {
            ch = 0;
            break;
        }

        if ((ch >= '0' && ch <= '9') || ch == '-' || ch == 8)
        {
            if (ch == 8)
            {
                if (i > 0)
                {
                    pthread_mutex_lock(&mutex);
                    gotoxy(mainMenuOptionPosition.X+len+i, mainMenuOptionPosition.Y);
                    printf(" ");
                    i--;
                    pthread_mutex_unlock(&mutex);
                }
            }
            else
            {
                pthread_mutex_lock(&mutex);
                gotoxy(mainMenuOptionPosition.X+len+i, mainMenuOptionPosition.Y);
                printf("%c", ch);
                pthread_mutex_unlock(&mutex);
                input[i] = ch;
                i++;
            }
        }
        ch = 0;
    }

    input[i] = '\0';
    sscanf(input, "%d", &num);

    return num;
}

int getNumberOfMultipleStopwatches()
{
    pthread_mutex_lock(&mutex);
    gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y);
    printf("                                                                ");
    gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y+1);
    printf("                                                                ");
    gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y+2);
    printf("                                                                ");
    gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y+3);
    printf("                                                                ");
    gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y+4);
    printf("                                                                ");
    char str[] = "Please, enter the number of stopwatches(2~10):";
    int len = strlen(str);
    pthread_mutex_unlock(&mutex);
    for(int i = 0; i < len; i++)
    {
        pthread_mutex_lock(&mutex);
        gotoxy(mainMenuOptionPosition.X+i, mainMenuOptionPosition.Y);
        putchar(str[i]);
        pthread_mutex_unlock(&mutex);
        delay(1);
    }
//    showcursor();
    int n;
    while(n = scanInt(len), n <= 1 || n > 10)
    {
        pthread_create(&thread2, NULL, showWrongInputError, NULL);
        pthread_mutex_lock(&mutex);
//        hidecursor();
        gotoxy(mainMenuOptionPosition.X+len, mainMenuOptionPosition.Y);
        printf("                             ");
        pthread_mutex_unlock(&mutex);
    }
    hidecursor();
    return n;
}

int executeMainMenuChoice1()
{
    return mainMenuChoice ? getNumberOfMultipleStopwatches() : 1;
}

void *startScreenDecoration();
void *loadingScreen();
int datapassDecoMain;
int messageBoxForegroundSignal = 0;

void *makeMessageBoxForeground()
{
    while(!messageBoxForegroundSignal);
    delay(500);
    HWND hwndMessageBox = FindWindow(NULL, "Warning");
    if (hwndMessageBox != NULL)
        SetForegroundWindow(hwndMessageBox);
    pthread_exit(NULL);
}

void mainMenu()
{
    mainMenuOptionPosition.X = 43;
    mainMenuOptionPosition.Y = 13;
//    pthread_create(&thread1, NULL, mainMenuOptionsChoiceInput, NULL);
    pthread_create(&thread1, NULL, startScreenDecoration, NULL);
    int choice = 0;
#define numChoice 3
    int print[numChoice] = {1, 1, 1};
    int len[numChoice+1];
    while(1)
    {
//        if(mainMenuChoiceDone)
//            break;
        if(ch == UP)
        {

            ch = 0;
            choice = ((choice-1)%numChoice + numChoice)%numChoice;
        }
        else if(ch == DOWN)
        {

            ch = 0;
            choice = (choice+1)%numChoice;
        }
        else if(ch == ENTER)
        {
            ch = 0;
            if(choice == 2)
            {
//                pthread_mutex_lock(&mutex);
//                stuckThread1 = 1;
//                delay(500);
                messageBoxForegroundSignal = 0;
                pthread_create(&thread2, NULL, makeMessageBoxForeground, NULL);
                messageBoxForegroundSignal = 1;
                int des = MessageBox(NULL, "Do you really want to quit?", "Warning", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
                if(des == IDYES)
                {
//                    pthread_cond_destroy(&cond);
                    pthread_mutex_destroy(&mutex);
                    pthread_cancel(thread1);
                    pthread_cancel(thread2);
                    pthread_join(thread1, NULL);
                    pthread_join(thread2, NULL);
                    system("cls");
                    exit(0);
                }
//                stuckThread1 = 0;
//                pthread_cond_signal(&cond);
//                pthread_mutex_unlock(&mutex);
            }
            else
                break;
        }
        if(choice < 2)
            mainMenuChoice = choice;
        pthread_mutex_lock(&mutex);
        gotoxy(mainMenuOptionPosition.X, mainMenuOptionPosition.Y);
        len[3] = printf("Select(Up/Down) then press Enter.");
        for(int i = 0; i < X-mainMenuOptionPosition.X-len[3]+1; i++)
            putchar(' ');
        pthread_mutex_unlock(&mutex);
        for(int i = 0; i < numChoice; i++)
            if(i == choice)
                print[i] = !print[i];
            else
                print[i] = 1;
        pthread_mutex_lock(&mutex);
        gotoxy(mainMenuOptionPosition.X+6, mainMenuOptionPosition.Y+1);
        if(print[0])
            len[0] = printf("Run Single Stopwatch");
        else
            len[0] = printf("                   ");
        for(int i = 0; i < X-mainMenuOptionPosition.X-6-len[0]+1; i++)
            putchar(' ');
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex);
        gotoxy(mainMenuOptionPosition.X+4, mainMenuOptionPosition.Y+2);
        if(print[1])
            len[1] = printf("Run Multiple Stopwatches");
        else
            len[1] = printf("                       ");
        for(int i = 0; i < X-mainMenuOptionPosition.X-4-len[1]+1; i++)
            putchar(' ');
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex);
        gotoxy(mainMenuOptionPosition.X+14, mainMenuOptionPosition.Y+3);
        if(print[2])
            len[2] = printf("Quit");
        else
            len[2] = printf("       ");
        for(int i = 0; i < X-mainMenuOptionPosition.X-14-len[2]+1; i++)
            putchar(' ');
        pthread_mutex_unlock(&mutex);
        int delayTime = 1;
        for(int i = 0; i < numChoice; i++)
            if(!print[i])
                delayTime = 0;
        if(!delayTime)
            delay(50);
        else
            delay(200); /// when both have been printed
    }
    stopwatches = executeMainMenuChoice1();
    pthread_create(&thread3, NULL, loadingScreen, NULL);
    pthread_cancel(thread1);
    pthread_cancel(thread2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_cancel(thread3);
    pthread_join(thread3, NULL);
}

int get_window_size(int* rows, int* cols)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleScreenBufferInfo(console, &csbi))
    {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return 1;
    }
    return 0;
}

void *startScreenDecoration()
{
    srand(time(NULL));
    while(1)
    {
        for (int row = 0; row < stpwchRow; row++)
        {
            pthread_testcancel();
            pthread_mutex_lock(&mutex);
            gotoxy(0, 3+row);
            for(int i = 0; i < 15; i++)
                putchar(' ');
            for (int i = 0; i < stpwchTot; i++)
            {
                for (int col = 0; col < stpwchCol; col++)
                {
//                    while(stuckThread1)
//                        pthread_cond_wait(&cond, &mutex);
                    if(col < stopwatchLen[i])
                    {
                        char cc = STOPWATCH[i][row][col];
                        if(cc == ' ')
                            putchar(' ');
                        else
                            putchar(rand()%10+'0');
                    }
                }
            }
            for(int i = 0; i < 20; i++)
                putchar(' ');
            pthread_mutex_unlock(&mutex);
        }
        delay(40);
    }
}

void *startScreenDecorationOptionsPre()
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
#define numStr 4
#define lenStr 37
    char mainMenuOptionsStr[numStr][lenStr] =
    {
        "\0 Select(Up/Down) then press Enter. ",
        "\0       Run Single Stopwatch        ",
        "\0     Run Multiple Stopwatches      ",
        "\0               Quit                "
    };
    for (int col = 0; col < lenStr-2; col++)
    {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        for (int row = 0; row < numStr; row++)
        {
            char tc = mainMenuOptionsStr[row][col];
            mainMenuOptionsStr[row][col] = mainMenuOptionsStr[row][col+1];
            mainMenuOptionsStr[row][col+1] = tc;
            gotoxy(X-col-1, 13+row);
            printf("%s", &mainMenuOptionsStr[row]);
        }
        pthread_mutex_unlock(&mutex);
        delay(28);
    }

    for (int x = lenStr-2; X-x >= 43; x++)
    {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        for (int row = 0; row < numStr; row++)
        {
            gotoxy(X-x-1, 13+row);
            printf("%s", &mainMenuOptionsStr[row]);
        }
        pthread_mutex_unlock(&mutex);
        delay(29);
    }
    pthread_exit(NULL);
}

void *startScreenDecorationPre()
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    srand(time(NULL));
    for (int col = 83; col > 0; col--)
    {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        for (int row = 0; row < 7; row++)
        {
            gotoxy(0, 3+row);
            printf("%s", &Stopwatch2d[row][col]);
        }
        pthread_mutex_unlock(&mutex);
        delay(15);
    }
    for (int x = 0; x < 15; x++)
    {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        for (int row = 0; row < 7; row++)
        {
            gotoxy(x, 3+row);
            printf("%s", &Stopwatch2d[row]);
        }
        pthread_mutex_unlock(&mutex);
        delay(15);
    }
    pthread_exit(NULL);
}

void *loadingScreen()
{
    char str[] = "Loading";
    int len = strlen(str);
    int row, col;
//    int des = get_window_size(&row, &col);
    while(1)
    {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        system("cls");
        for(int k = 0; k < loadingCol; k++)
        {
            int x = k;
            for(int i = 0; i < loadingTot; i++)
            {
                for(int j = 0; j < loadingRow; j++)
                {
                    gotoxy(30+x, 10+j);
                    if(k < loadingLen[i])
                        putchar(LOADING[i][j][k]);
                }
                x += loadingLen[i]+2;
            }
            usleep(1);
        }
        pthread_mutex_unlock(&mutex);
    }
}

#define fromMainMenu 0
#define fromReset 1

//void *startScreenSkip()
//{
//    while(1)
//    {
//        pthread_testcancel();
//        pthread_mutex_lock(&mutex);
//        gotoxy(93, 0);
//        printf("Press any key to skip >>>");
//        pthread_mutex_unlock(&mutex);
//        delay(200);
//        pthread_mutex_lock(&mutex);
//        gotoxy(93, 0);
//        printf("                         ");
//        pthread_mutex_unlock(&mutex);
//        delay(50);
//    }
//}

void resetValuesForMainMenu()
{
    system("cls");
    ch = 0;
    hidecursor();
    stuckThread1 = 0;
    pthread_mutex_lock(&mutex);
    gotoxy(93, 0);
    printf("Press any key to skip >>>");
    pthread_mutex_unlock(&mutex);
//    pthread_create(&thread3, NULL, startScreenSkip, NULL);
    pthread_create(&thread1, NULL, startScreenDecorationOptionsPre, NULL);
    pthread_create(&thread2, NULL, startScreenDecorationPre, NULL);
    while(!ch && (!pthread_kill(thread1, 0) || !pthread_kill(thread2, 0)))
        delay(100);
    pthread_mutex_lock(&mutex);
    gotoxy(93, 0);
    printf("                         ");
    pthread_mutex_unlock(&mutex);
    pthread_cancel(thread1);
    pthread_cancel(thread2);
//    pthread_cancel(thread3);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
//    pthread_join(thread3, NULL);
    ch = 0;
    mainMenuChoice = 0;
    mainMenuChoiceDone = 0;
    endStopwatchRunning = 0;
    liveResultAsc = 1;
}

void resetValuesForReset()
{
    ch = 0;
    startStopwatch = 0;
    stopStopwatch = 1;
    focusNo = 0;
    endStopwatchRunning = 0;
    liveResultAsc = 1;
}

int main()
{
    hidecursor();
    _getch();
    delay(5000);
    get_window_size(&Y, &X);
    pthread_create(&inputThread, NULL, inputFunction, NULL);
    int flowDecision = 0;
    while(1)
    {
        switch(flowDecision)
        {
        case fromMainMenu:
            resetValuesForMainMenu();
            mainMenu();
        case fromReset:
            resetValuesForReset();
            pthread_create(&thread1, NULL, StartPageOfStopwatches, &stopwatches);
            flowDecision = -1;
        }
        while(1)
        {
            if(flowDecision == fromMainMenu || flowDecision == fromReset)
            {
                pthread_create(&thread3, NULL, loadingScreen, NULL);
                pthread_cancel(thread1);
                pthread_cancel(thread2);
                pthread_join(thread1, NULL);
                pthread_join(thread2, NULL);
                pthread_cancel(thread3);
                pthread_join(thread3, NULL);
                startStopwatch = 0;
                stopStopwatch = 1;
                break;
            }
//            ch = _getch();
            if(mainMenuChoice && ch >= '0' && ch <= '9')
                mulSkip[ch - '0']++, ch = 0;
            else if(mainMenuChoice && ch == 'o')
            {
                ch = 0, liveResultAsc = !liveResultAsc;
                if(endStopwatchRunning)
                {
                    pthread_join(thread2, NULL);
                    pthread_create(&thread2, NULL, printLiveResult, NULL);
                }
            }
            else if(!startStopwatch && ch == ' ')
                ch = 0, startStopwatch = 1;
            else if(ch == 'r')
                ch = 0, flowDecision = fromReset;
            else if(ch == 'm')
                ch = 0, flowDecision = fromMainMenu;
//            else if(ch == 'e')
//            {
//                ch = 0;
//                if(MessageBox(NULL, "Do you really want to quit?", "Warning", MB_ICONWARNING | MB_YESNO) == IDYES)
//                    exit(0);
//            }
        }
    }
    return 0;
}
