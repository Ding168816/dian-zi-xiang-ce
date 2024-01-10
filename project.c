#include <stdio.h>                 //引入标准输入输出头文件
#include <sys/types.h>            //引入系统类型头文件
#include <sys/stat.h>            //引入系统状态头文件
#include <fcntl.h>              //引入文件控制头文件
#include <unistd.h>            //引入Unix标准头文件
#include <string.h>           //引入字符串处理头文件
#include <sys/mman.h>        //引入内存映射头文件
#include <stdlib.h>         //引入标准库头文件
#include <time.h>          //引入时间库头文件
#include <linux/input.h>  

#define MOVE_UP 1
#define MOVE_DOWE 2 
#define MOVE_LEFT 3
#define MOVE_RIGHT 4
#define BTN_TOUCH 0x14a //触摸屏按键
#define ABS_PRESSURE 0x18 //压力
int ret_value ;


int *plcd = NULL;         //指向帧缓冲首地址
int fd_lcd = -1;         //显示屏的文件描述符
void srandom(unsigned int seed);//声明生成随机种子数函数

void LCD_Draw_Point(int x, int y , int color)            //定义一个函数LCD_Draw_Point，用于在LCD屏幕上画一个点，接受三个参数：x坐标，y坐标和颜色值
{ 
	if(x >= 0 && x < 800 && y >= 0 && y < 480)               //判断坐标是否在屏幕范围内（800*480）
	{
		*(plcd + 800*y + x) = color;                        //如果是，则计算出对应的帧缓冲地址，并赋值为颜色值
	}
	
}

char * mbuf[] = {"/DYS/01.bmp","/DYS/02.bmp","/DYS/03.bmp","/DYS/04.bmp","/DYS/05.bmp","/DYS/06.bmp"};//定义一个字符指针数组mbuf，初始化为一张bmp图片的路径名 
void Bmp_Display(int x0, int y0, char * bmp_pathname )       //定义一个函数Bmp_Display，用于在LCD屏幕上显示一张bmp图片，接受三个参数：起始x坐标，起始y坐标和图片路径名 
{
//1.打开图片文件 
	int fd_bmp = open(bmp_pathname, O_RDONLY);                  //调用open函数以只读方式打开图片文件，并返回一个文件描述符fd_bmp,pathname是指针
	if(fd_bmp == -1)                                           //判断是否打开成功 
	{
		perror("Open bmp error");                            //如果失败，则打印错误信息并退出 
	}

//2.读取此文件的 宽、高、色深
	int width, height;                                    //定义两个整型变量width和height，用于存储图片的宽度和高度 
	short depth;                                         //定义一个短整型变量depth，用于存储图片的色深 
	unsigned char buf[4];                               //定义一个无符号字符数组buf，大小为4字节，用于临时存储读取到的数据 
	lseek(fd_bmp, 0x12 , SEEK_SET);                    //调用lseek函数将文件指针移动到偏移量为0x12（18）的位置（即宽度数据开始的位置） 
	int ret = read(fd_bmp, buf , 4 );                 //调用read函数从fd_bmp中读取4个字节（即宽度数据）到buf中，并返回实际读取到的字节数ret 
	if(ret == -1)                                    //判断是否读取成功 
	{
		perror("Read width error");                 //如果失败，则打印错误信息并关闭fd_bmp 
		close(fd_bmp);
	}
	width = (buf[3] << 24) |                      //将buf中的四个字节按照小端模式拼接成一个整数赋值给width（即宽度）
			(buf[2] << 16) |  
			(buf[1] << 8)  |  
			(buf[0]) ; 


	lseek(fd_bmp, 0x16 , SEEK_SET);              //调用lseek函数将文件指针移动到偏移量为0x16（22）的位置（即高度数据开始的位置）  
	ret= read(fd_bmp, buf , 4 );                //调用read函数从fd_bmp中读取4个字节（即高度数据）到buf中，并返回实际读取到的字节数ret  
	if(ret == -1)                              //判断是否读取成功 
	{ 
		perror("Read height error");         //如果失败，则打印错误信息并关闭fd_bmp 
		close(fd_bmp);
	}
	height = (buf[3] << 24) |              //将buf中的四个字节按照小端模式拼接成一个整数赋值给height（即高度）
			(buf[2] << 16) |  
			(buf[1] << 8)  |  
			(buf[0]) ; 

	lseek(fd_bmp, 0x1C , SEEK_SET);    //调用lseek函数将文件指针移动到偏移量为0x1C（28）的位置（即色深数据开始的位置）  
	ret= read(fd_bmp, buf , 2 );      //调用read函数从fd_bmp中读取2个字节（即色深数据）到buf中，并返回实际读取到的字节数ret  
	if(ret == -1)                    //判断是否读取成功 
	{
		perror("Read depth error"); //如果失败，则打印错误信息并关闭fd_bmp 
		close(fd_bmp);
	}
	
	depth = (buf[1] << 8) |   //将buf中的两个字节按照小端模式拼接成一个短整数赋值给depth（即色深）
			buf[0]; 

	printf("width = %d, height = %d, depth = %d\n", 
			width, height, depth);

	int laizi  = 0;                                           //初试话赖子个数
	int line_bytes = abs(width) * depth/8 ;                  //一行有效字节数 
	if(line_bytes % 4 != 0)
	{
		laizi = 4  - line_bytes % 4;
	}
	int bytes = line_bytes + laizi ;                        //一行总字节数
	int arr_size = bytes  * abs(height);
	unsigned char * p =	malloc(arr_size);

	lseek(fd_bmp, 54 , SEEK_SET);
	int r0 = read(fd_bmp, p , arr_size );                  //读像素数组
	if(r0 == -1)
	{
		perror("Read arr error");
		close(fd_bmp);
	}
	unsigned char a,r,g,b;
			int i=0; 
			int color; 
			int x, y;
			for(y = 0; y <abs(height) ;  y++ )
			{
				for(x = 0 ; x <  abs(width) ; x++ )
				{
					b = *(p + i); 
					i++;
					g = *(p + i); 
					i++;
					r = *(p + i);
					i++;
					if(depth == 24) //RGB 
					{
						a = 0x00;
					}
					else //ARGB 
					{
						a = *(p + i);
						i++;
					}
					color = (a << 24) | (r << 16) | (g << 8) | b ;
					LCD_Draw_Point( width > 0 ?   x0 + x : x0+abs(width)-1-x  , 
									height > 0 ?  y0+abs(height)-1-y : y0 + y ,color);
				}
				i += laizi ; //跳过赖子
			} 
			
			close(fd_bmp);
			free(p);
}


/*
	LCD_Init : 初始化显示屏
	参数: 无 
	返回值: 无
*/
void LCD_Init(void)
{
	//1.打开屏幕文件
	int fd_lcd = open("/dev/fb0",O_RDWR);
	if (fd_lcd == -1)                             // 如果打开失败（返回值为 -1）
	{ 
        perror("Open lcd Error:");              // 使用 perror() 函数打印错误信息
    } 


	//2.映射
	plcd=mmap(NULL,800*480*4,PROT_READ | PROT_WRITE ,MAP_SHARED,fd_lcd,0);//调用mmap函数将fd_lcd所指向的帧缓冲区映射到用户空间，并返回一个整型指针plcd指向映射后的首地址 
	if(plcd == MAP_FAILED)
	{
		perror("Mmap Error");
		close(fd_lcd);
	}
}
/*
	LCD_Uninit : 显示屏解初始化
	参数: 无 
	返回值: 无
*/

void LCD_Uninit(void)
{
	//4.解映射 
	munmap(plcd , 800*480*4);

	//5.关闭文件 
	close(fd_lcd);
}



/*
			Move_Direction : 获取手指滑动的方向
			参数: 无 
			返回值: 
				返回滑动的方向 int
		*/
		
int Move_Direction()  //滑动文件函数
{
	int x1 = -1, y1 = -1; //起点坐标 
	int x2, y2; //终点坐标
	

	//1.打开触目屏的文件
	int fd_touch = open("/dev/input/event0",O_RDWR);
	if (fd_touch == -1)                               // 如果打开失败（返回值为 -1）
	{ 
        perror("Open fd_touch Error:");              // 使用 perror() 函数打印错误信息
        return -1;                                  // 返回 -1 表示程序异常结束
    } 

 	
	//2.不断获取坐标点的值 
	while(1)
	{
		struct  input_event ev; //输入事件结构体 
		int ret = read(fd_touch , &ev , sizeof(ev) );
		if(ret != sizeof(ev) )
		{
			//读到的信息有误 
			continue;
		}
		
		if(ev.type == EV_ABS && ev.code == ABS_X )
		{
			//发过来的是X轴
			if(x1 == -1)
			{
				x1 = ev.value; 
			}
			x2 = ev.value ;
		}
		
		if(ev.type == EV_ABS && ev.code == ABS_Y )
		{
			//发过来的是X轴
			if(y1 == -1)
			{
				y1 = ev.value; 
			}
			y2 = ev.value;
		}
		
		//判断滑动结束
		if(  (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0) ||
			 (ev.type == EV_ABS && ev.code == ABS_PRESSURE && ev.value == 0) )
		{
			//解析滑动的方向，进行返回
			int delt_x = abs(x1 - x2);
			int delt_y = abs(y1 - y2);
			
			
			 
			if(delt_x > delt_y)
			{
				//说明肯定是左右滑动 
				if(x1 > x2)
				{
					ret_value = MOVE_LEFT;
					return 3;
					break;
				}
				else 
				{
					ret_value = MOVE_RIGHT ;
					return 4;
					break;
				}
				
			}
			else if(delt_x < delt_y) 
			{
				if(y1>y2)
				{
					ret_value = MOVE_UP;
					return 1;
					break;
				}
			else
				{
					ret_value = MOVE_DOWE;
					return 2;
					break;
				} 
			}
			else
				{
				//滑动无效 
				continue;
				}
		}
	}
	
	close(fd_touch);
	return ret_value;
}


int main(int argc, char * argv [ ])
{	LCD_Init();                                                                        //调用初始化显示屏函数
	int num;
	printf("1.随机播放\n2.顺序播放\n3.滑动播放\n\n请选择播放模式：");                                     //从键盘输入数字1-3选择你想需要播放模式
	scanf("%d",&num);
	int i;
	if(num==1)
		{
			//3.1随机播放
			Bmp_Display(0,0, mbuf[i]); 
			while(1)
			{	i = random() % 6 ;          //保证i属于[0, n-1]
				srandom( time(NULL) );     //设置随机数种子
				Bmp_Display(0,0,mbuf[i]); 
				sleep(1);
			}
		}

	else if(num==2)
		{
	   		//3.2顺序播放
			i = 0;
			while(1)
			{
				Bmp_Display(0,0, mbuf[i]);
				sleep(1); //man -f sleep
				i++; 
				if(i == 5)
				{
					i = 0;
				}
			}
		}
	else if(num==3)
		{	//3.3滑动播放
			i=0; 
			while (1)
		   {
				int fx = Move_Direction();
				if(fx == 1||fx == 3)
					{
							if(i == 5)
							
								{
									i=0;
								}
							else
								{
									++i;
									Bmp_Display(0, 0,mbuf[i]);
								}	
					}	
				else if(fx == 2||fx == 4)
					{
						
							if(i == 0)
								{
									i=5;
								}
							else
								{
									--i;
									Bmp_Display(0, 0,mbuf[i]);
								}
							
					}
		   }
		}

	LCD_Uninit();       //调用显示屏,解映射，初始化函数	
}














































