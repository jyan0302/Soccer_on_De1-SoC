#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

struct endPoints {
    int startX;
    int startY;
    int endX;
    int endY;
};

struct pointArray {
    int xPoints [320];
    int yPoints [320];
    int size;
};

struct hitBox {
    int minX;
    int maxX;
    int minY;
    int maxY;
};

typedef struct playerstat {
    int x;
    int y;
    int vel_x;
    int vel_y;
    int x_min;
    int x_max;
    int y_min;
    int y_max;
    int dir_x;
    int dir_y;
} player;

//320*240

volatile int pixel_buffer_start; // global variable
void clear_screen();

void plot_pixel(int x, int y, short int line_color);
void wait_for_vsync();
void draw_person (int x, int y, short int color);
void draw_ball (int x, int y, short int color);
void draw_goal(int x, int y, int width, short int color);
struct endPoints trajPoints (double angle, int x,int y);
struct pointArray draw_line (int x1, int y1, int x2, int y2, short int color);
bool doesIntercept (struct hitBox hitbox, int x, int y);
struct hitBox make_hitBox (int x, int y);
void write_hex (int code);
void reverseArray(int arr[], int start, int end);
void level1(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex);
void level2(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex);
void level3(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex);
void level4(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex);

int main(void) {
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; 
    pixel_buffer_start = *(pixel_ctrl_ptr+1);
    clear_screen();
    wait_for_vsync();
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();

    volatile int * key_edge_address = (int *)0xFF20005C;
    volatile char* key_edge_address2 = (char *)0xFF20005C;

    //colors
    int black = 0x0;
    int blue = 0x001F;
    int red = 0xF800;
    int green = 0x07E0;
    int pink = 0xF81F;
    int white = 0xFFFF;
    int lightblue = 0x2353;

    //code for hexes
    const int hexgoal = 1;
    const int hexfailed = 2;
    const int hexshoot = 3;

    //constants
    const int headtoballx = 10;
    const int headtobally = 34;
    const int goalpostleftx = 98;
    const int goalposty = 0;
    const int goalpostwidth = 125;

    //goal hitbox
    struct hitBox goal;
    goal.minX = 98;
    goal.maxX = 223;
    goal.minY = 0;
    goal.maxY = 20;

    //teammates
    int allynum;
    player ally[20];

    //opponents
    int foenum;
    player foe[20];

    //ball pos
    int curBallPosX;
    int curBallPosY;

    //params
    int ignoreAlly;
    bool shoot;
    int angle;
    int balltrajindex;

    //trajectory struct
    struct endPoints traj_line;
    struct pointArray traj_array;

    int levelnum = 1;
    bool changelevel = true;
    int maxlevel = 4;

    //once you freeze, then only way to unfreeze to is press key 2 or 3
    bool freeze = false;

    
    //game loop
    while (true){
        
        if (changelevel){
            freeze = false;
            if (levelnum==1){
                level1(&allynum, ally, &foenum, foe, &curBallPosX, &curBallPosY,
                    &ignoreAlly, &shoot, &angle, &balltrajindex);
            } else if (levelnum==2){
                level2(&allynum, ally, &foenum, foe, &curBallPosX, &curBallPosY,
                    &ignoreAlly, &shoot, &angle, &balltrajindex);
            } else if (levelnum==3){
                level3(&allynum, ally, &foenum, foe, &curBallPosX, &curBallPosY,
                    &ignoreAlly, &shoot, &angle, &balltrajindex);
            } else if (levelnum==4){
                level4(&allynum, ally, &foenum, foe, &curBallPosX, &curBallPosY,
                    &ignoreAlly, &shoot, &angle, &balltrajindex);
            }
            changelevel=false;
        }

        while (true){
            //check for key inputs
            char input = *(key_edge_address2);
            *(key_edge_address)=*(key_edge_address);
            if (input>0){
                if (input>0b111){
                    //reset
                    changelevel = true;
                    levelnum = 1;
                    break;
                } else if (input>0b11){
                    if (freeze){
                        //next level
                        if (levelnum!=maxlevel){
                            changelevel = true;
                            levelnum++;
                            break;
                        }
                    } else {
                        //shoot
                        shoot = true;
                    }
                } else if (input>0b1){
                    angle+=6;
                } else {
                    angle -=4;
                }
                
            }
            if (!freeze){
                clear_screen();

                //draw goal
                draw_goal(goalpostleftx,goalposty,goalpostwidth,red);

                /*draw players */    
                { 
                    for (int i = 0; i<allynum;i++){
                        if (ally[i].dir_x==1){
                            //moving in position dir
                            if (ally[i].x<ally[i].x_max){
                                ally[i].x += ally[i].vel_x;
                            } else {
                                ally[i].x -= ally[i].vel_x;
                                ally[i].dir_x = -1;
                            }
                        } else if (ally[i].dir_x==-1){
                            //moving in negative dir
                            if (ally[i].x>ally[i].x_min){
                                ally[i].x -= ally[i].vel_x;
                            } else {
                                ally[i].x += ally[i].vel_x;
                                ally[i].dir_x = 1;
                            }
                        }
                        if (ally[i].dir_y==1){
                            //moving in position dir
                            if (ally[i].y<ally[i].y_max){
                                ally[i].y += ally[i].vel_y;
                            } else {
                                ally[i].y -= ally[i].vel_y;
                                ally[i].dir_y = -1;
                            }
                        } else if (ally[i].dir_y==-1){
                            //moving in negative dir
                            if (ally[i].y>ally[i].y_min){
                                ally[i].y -= ally[i].vel_y;
                            } else {
                                ally[i].y += ally[i].vel_y;
                                ally[i].dir_y = 1;
                            }
                        }
                        draw_person(ally[i].x,ally[i].y,pink);
                    }
                    for (int i = 0; i<foenum;i++){
                        if (foe[i].dir_x==1){
                            //moving in position dir
                            if (foe[i].x<foe[i].x_max){
                                foe[i].x += foe[i].vel_x;
                            } else {
                                foe[i].x -= foe[i].vel_x;
                                foe[i].dir_x = -1;
                            }
                        } else if (foe[i].dir_x==-1){
                            //moving in negative dir
                            if (foe[i].x>foe[i].x_min){
                                foe[i].x -= foe[i].vel_x;
                            } else {
                                foe[i].x += foe[i].vel_x;
                                foe[i].dir_x = 1;
                            }
                        }
                        if (foe[i].dir_y==1){
                            //moving in position dir
                            if (foe[i].y<foe[i].y_max){
                                foe[i].y += foe[i].vel_y;
                            } else {
                                foe[i].y -= foe[i].vel_y;
                                foe[i].dir_y = -1;
                            }
                        } else if (foe[i].dir_y==-1){
                            //moving in negative dir
                            if (foe[i].y>foe[i].y_min){
                                foe[i].y -= foe[i].vel_y;
                            } else {
                                foe[i].y += foe[i].vel_y;
                                foe[i].dir_y = 1;
                            }
                            
                        }
                        draw_person(foe[i].x,foe[i].y,lightblue);
                    }
                    
                }

                if (shoot){
                    //ball is moving
                    //check if out of bounds
                    if (balltrajindex >= traj_array.size){
                        write_hex(hexfailed);
                        freeze = true;
                        draw_ball(curBallPosX, curBallPosY, white);
                    } else {
                        curBallPosX = traj_array.xPoints[balltrajindex];
                        curBallPosY = traj_array.yPoints[balltrajindex];
                        draw_ball(curBallPosX, curBallPosY, white);
                        balltrajindex+=15;
                        
                        //make hitboxes
                        struct hitBox allyhitbox [allynum];
                        struct hitBox foehitbox [foenum];

                        //check for collisons
                        for (int i=0; i<allynum;i++){
                            allyhitbox[i]=make_hitBox(ally[i].x,ally[i].y);
                        }
                        for (int i=0; i<foenum;i++){
                            foehitbox[i]=make_hitBox(foe[i].x,foe[i].y);
                        }
                        //check for blocks
                        for (int i=0; i<foenum;i++){
                            if (doesIntercept(foehitbox[i],curBallPosX,curBallPosY)){
                                write_hex(hexfailed);
                                freeze = true;
                            }
                        }
                        //check for passes
                        for (int i=0; i<allynum;i++){
                            if (doesIntercept(allyhitbox[i],curBallPosX,curBallPosY)&&(ignoreAlly!=i)){
                                curBallPosX = ally[i].x+headtoballx;
                                curBallPosY = ally[i].y+headtobally;
                                ally[i].dir_x = 0;
                                ally[i].dir_y = 0;
                                ignoreAlly = i;
                                shoot = false;
                                angle = 20;
                            }
                        }
                        //check for goals
                        if (doesIntercept(goal,curBallPosX,curBallPosY)){
                            write_hex(hexgoal);
                            freeze = true;
                        }
                    }
                } else {
                    draw_ball(curBallPosX, curBallPosY, white);
                    traj_line = trajPoints(angle,curBallPosX,curBallPosY);
                    traj_array = draw_line(traj_line.startX,traj_line.startY,traj_line.endX,traj_line.endY,green);
                    balltrajindex = 0;
                }

                //draw
                wait_for_vsync(); // swap front and back buffers on VGA vertical sync
                pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
            }//freeze loop
        }//level loop
    }//game loop
}//main

struct pointArray draw_line (int x1, int y1, int x2, int y2, short int color){
    struct pointArray returnArray;
    bool isSteep = abs(y2-y1) > abs(x2-x1);
    
    //swap x and y
    if (isSteep){
        int temp = x1;
        x1 = y1;
        y1 = temp;
        temp = x2;
        x2 = y2;
        y2 = temp;
    } 
    bool swap = x1>x2;
    //swap forwards and backwards
    if (swap){
        int temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;
    }

    //Bresenham’s algorithm
    int deltaX = x2-x1;
    int deltaY = abs(y2-y1);
    int error = deltaX*-(1/2);
    int yStep;
    if (y1<y2) {
        yStep = 1;
    } else {
        yStep = -1;
    }
    int y = y1;
    int i=0;
    for (int x=x1; x<=x2; x++){
        if (isSteep){
            plot_pixel (y,x,color);
            returnArray.xPoints[i] = y;
            returnArray.yPoints[i] = x;
        } else {
            plot_pixel (x,y,color);
            returnArray.xPoints[i] = x;
            returnArray.yPoints[i] = y;
        }
        error = error + deltaY;
        if (error>=0){
            y = y+yStep;
            error = error - deltaX;
        }
        i++;
        returnArray.size = i;
    }
    if (swap){
        reverseArray(returnArray.xPoints,0,returnArray.size-1);
        reverseArray(returnArray.yPoints,0,returnArray.size-1);
    }
    return returnArray;
}

//prints black pixels on the entire screen
void clear_screen(){
    //screen is 320*240 pixels
    for (int x=0; x<320; x++){
        //x is columns, goes left to right
        for (int y=0; y<240; y++){
            plot_pixel(x,y,0);
        }
    }
}

void plot_pixel(int x, int y, short int line_color){
    if ((x>319)||(x<0)||(y>239)||(y<0)){
        return;
    }
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    register int status;
    *pixel_ctrl_ptr = 1;        //tells buffers to switch
    status = *(pixel_ctrl_ptr+3); //checks when print has ended
    while ((status&0x01)!=0){   //while print has not ended, loop
        status = *(pixel_ctrl_ptr+3);
    }
}

void draw_person(int x, int y, short int color){
    //draw head 
    //radius 8 pixels
    plot_pixel(x,y-8,color);
    plot_pixel(x+1,y-8,color);
    plot_pixel(x+2,y-8,color);
    plot_pixel(x+3,y-8,color);
    plot_pixel(x+4,y-7,color);
    plot_pixel(x+5,y-6,color);
    plot_pixel(x+5,y-6,color);
    plot_pixel(x+5,y-5,color);
    plot_pixel(x+6,y-4,color);
    plot_pixel(x+6,y-3,color);
    plot_pixel(x+7,y-2,color);
    plot_pixel(x+7,y-1,color);
    plot_pixel(x+7,y,color);
    plot_pixel(x+7,y+1,color);
    plot_pixel(x+6,y+2,color);
    plot_pixel(x+6,y+3,color);
    plot_pixel(x+5,y+4,color);
    plot_pixel(x+5,y+5,color);
    plot_pixel(x+4,y+5,color);
    plot_pixel(x+3,y+6,color);
    plot_pixel(x+2,y+6,color);
    plot_pixel(x+1,y+7,color);
    plot_pixel(x,y+7,color);
    //reflected across x
    plot_pixel(x,y-8,color);
    plot_pixel(x-1,y-8,color);
    plot_pixel(x-2,y-8,color);
    plot_pixel(x-3,y-8,color);
    plot_pixel(x-4,y-7,color);
    plot_pixel(x-5,y-6,color);
    plot_pixel(x-5,y-6,color);
    plot_pixel(x-5,y-5,color);
    plot_pixel(x-6,y-4,color);
    plot_pixel(x-6,y-3,color);
    plot_pixel(x-7,y-2,color);
    plot_pixel(x-7,y-1,color);
    plot_pixel(x-7,y,color);
    plot_pixel(x-7,y+1,color);
    plot_pixel(x-6,y+2,color);
    plot_pixel(x-6,y+3,color);
    plot_pixel(x-5,y+4,color);
    plot_pixel(x-5,y+5,color);
    plot_pixel(x-4,y+5,color);
    plot_pixel(x-3,y+6,color);
    plot_pixel(x-2,y+6,color);
    plot_pixel(x-1,y+7,color);
    plot_pixel(x,y+7,color);
    //draw torso
    plot_pixel(x,y+8,color);
    plot_pixel(x,y+9,color);
    plot_pixel(x,y+10,color);
    plot_pixel(x,y+11,color);
    plot_pixel(x,y+12,color);
    plot_pixel(x,y+13,color);
    plot_pixel(x,y+14,color);
    plot_pixel(x,y+15,color);
    plot_pixel(x,y+16,color);
    plot_pixel(x,y+17,color);
    plot_pixel(x,y+18,color);
    plot_pixel(x,y+19,color);
    plot_pixel(x,y+20,color);
    plot_pixel(x,y+21,color);
    plot_pixel(x,y+22,color);
    plot_pixel(x,y+23,color);
    plot_pixel(x,y+24,color);
    plot_pixel(x,y+25,color);
    plot_pixel(x,y+26,color);
    plot_pixel(x,y+27,color);
    plot_pixel(x,y+28,color);
    plot_pixel(x,y+29,color);
    plot_pixel(x,y+30,color);
    plot_pixel(x,y+31,color);
    //draw right arm
    plot_pixel(x+1,y+15,color);
    plot_pixel(x+2,y+16,color);
    plot_pixel(x+3,y+17,color);
    plot_pixel(x+4,y+18,color);
    plot_pixel(x+5,y+18,color);
    plot_pixel(x+6,y+19,color);
    //draw left arm
    plot_pixel(x-1,y+15,color);
    plot_pixel(x-2,y+16,color);
    plot_pixel(x-3,y+17,color);
    plot_pixel(x-4,y+18,color);
    plot_pixel(x-5,y+18,color);
    plot_pixel(x-6,y+19,color);
    //draw right leg
    plot_pixel(x+1,y+31,color);
    plot_pixel(x+2,y+32,color);
    plot_pixel(x+3,y+33,color);
    plot_pixel(x+4,y+34,color);
    plot_pixel(x+5,y+34,color);
    plot_pixel(x+6,y+35,color);
    //draw left leg
    plot_pixel(x-1,y+31,color);
    plot_pixel(x-2,y+32,color);
    plot_pixel(x-3,y+33,color);
    plot_pixel(x-4,y+34,color);
    plot_pixel(x-5,y+34,color);
    plot_pixel(x-6,y+35,color);
}

void draw_ball (int x, int y, short int color){
    //radius 3 pixels
    plot_pixel(x,y+3,color);
    plot_pixel(x+1,y+3,color);
    plot_pixel(x+2,y+2,color);
    plot_pixel(x+3,y+1,color);
    plot_pixel(x+3,y,color);
    plot_pixel(x+3,y-1,color);
    plot_pixel(x+2,y-2,color);
    plot_pixel(x+1,y-3,color);
    plot_pixel(x,y-3,color);
    //reflect across x axis
    plot_pixel(x,y+3,color);
    plot_pixel(x-1,y+3,color);
    plot_pixel(x-2,y+2,color);
    plot_pixel(x-3,y+1,color);
    plot_pixel(x-3,y,color);
    plot_pixel(x-3,y-1,color);
    plot_pixel(x-2,y-2,color);
    plot_pixel(x-1,y-3,color);
    plot_pixel(x,y-3,color);
}

void draw_goal(int x, int y, int width, short int color){
    
    //left goalpost
    plot_pixel(x,y,color);
    plot_pixel(x,y+1,color);
    plot_pixel(x,y+2,color);
    plot_pixel(x,y+3,color);
    plot_pixel(x,y+4,color);
    plot_pixel(x,y+5,color);
    plot_pixel(x,y+6,color);
    plot_pixel(x,y+7,color);
    plot_pixel(x,y+8,color);
    plot_pixel(x,y+9,color);
    plot_pixel(x,y+10,color);
    plot_pixel(x,y+11,color);
    plot_pixel(x,y+12,color);
    plot_pixel(x,y+13,color);
    plot_pixel(x,y+14,color);
    plot_pixel(x,y+15,color);
    plot_pixel(x,y+16,color);
    plot_pixel(x,y+17,color);
    plot_pixel(x,y+18,color);
    plot_pixel(x,y+19,color);
    plot_pixel(x,y+20,color);
    plot_pixel(x,y+21,color);
    plot_pixel(x,y+22,color);
    plot_pixel(x,y+23,color);
    plot_pixel(x,y+24,color);
    plot_pixel(x,y+25,color);
    plot_pixel(x,y+26,color);
    plot_pixel(x,y+27,color);
    plot_pixel(x,y+28,color);
    plot_pixel(x,y+29,color);
    plot_pixel(x,y+30,color);
    plot_pixel(x,y+31,color);
    //right goalpost
    plot_pixel(x+width,y,color);
    plot_pixel(x+width,y+1,color);
    plot_pixel(x+width,y+2,color);
    plot_pixel(x+width,y+3,color);
    plot_pixel(x+width,y+4,color);
    plot_pixel(x+width,y+5,color);
    plot_pixel(x+width,y+6,color);
    plot_pixel(x+width,y+7,color);
    plot_pixel(x+width,y+8,color);
    plot_pixel(x+width,y+9,color);
    plot_pixel(x+width,y+10,color);
    plot_pixel(x+width,y+11,color);
    plot_pixel(x+width,y+12,color);
    plot_pixel(x+width,y+13,color);
    plot_pixel(x+width,y+14,color);
    plot_pixel(x+width,y+15,color);
    plot_pixel(x+width,y+16,color);
    plot_pixel(x+width,y+17,color);
    plot_pixel(x+width,y+18,color);
    plot_pixel(x+width,y+19,color);
    plot_pixel(x+width,y+20,color);
    plot_pixel(x+width,y+21,color);
    plot_pixel(x+width,y+22,color);
    plot_pixel(x+width,y+23,color);
    plot_pixel(x+width,y+24,color);
    plot_pixel(x+width,y+25,color);
    plot_pixel(x+width,y+26,color);
    plot_pixel(x+width,y+27,color);
    plot_pixel(x+width,y+28,color);
    plot_pixel(x+width,y+29,color);
    plot_pixel(x+width,y+30,color);
    plot_pixel(x+width,y+31,color);
}

/* this function determines the endpoints for a given angle from 
   a given position */
struct endPoints trajPoints (double angle, int x,int y){
    const double PI = 3.14159526;
    double rad = angle*PI/180;
    if (angle<90){
        double value = tan(rad);
        int maxXVal = 320-x;
        struct endPoints newPoints;
        newPoints.startX = x;
        newPoints.startY = y;
        for (double i=0;i<=maxXVal; i+=0.1){
            if (value*i>y){
                break;
            } else {
                newPoints.endX = ceil(x+i);
                newPoints.endY = floor(y-value*i);
            }
        }
        return newPoints;
    } else {
        double value = tan(rad)*-1;
        int maxXVal = x;
        struct endPoints newPoints;
        newPoints.startX = x;
        newPoints.startY = y;
        for (double i=0;i<=maxXVal; i+=0.1){
            if (value*i>y){
                break;
            } else {
                newPoints.endX = ceil(x-i);
                newPoints.endY = floor(y-value*i);
            }
        }
        return newPoints;
    }
}

/* this function checks if any points within the lineArray (aka trajectory) 
  exists within the hitbox */
bool doesIntercept (struct hitBox hitbox, int x, int y){
    if ((x>hitbox.minX)&&(x<hitbox.maxX)){
        if ((y>hitbox.minY)&&(y<hitbox.maxY)){
            return true;
        }
    }
    //not in hitbox
    return false;
}

/* this function creates a player hitbox around the given coordinates 
   player head */

struct hitBox make_hitBox (int x, int y){
    const int hitboxX = 8;
    const int hitboxYabove = 8;
    const int hitboxYbelow = 37;
    struct hitBox player2hitbox;
        player2hitbox.minX = x-hitboxX;
        player2hitbox.maxX = x+hitboxX;
        player2hitbox.minY = y-hitboxYabove;
        player2hitbox.maxY = y+hitboxYbelow;
    return player2hitbox;
}

void write_hex (int code){
    volatile int * hex_base_address1 = (int*) 0xFF200020;
    volatile int * hex_base_address2 = (int*) 0xFF200030;
    const int hexgoal = 1;
    const int hexfailed = 2;
    if (code==hexfailed){
        *(hex_base_address2)=0b0111000101110111;    //FA
        *(hex_base_address1)=0b00110000001110000111100101011110;    //ILED
    } else if (code==hexgoal){
        *(hex_base_address2)=0b0;    //Blank
        *(hex_base_address1)=0b01111101001111110111011100111000;    //GOAL
    } else {
        *(hex_base_address2) = 0b01101101; //S
        *(hex_base_address1)=0b01110110001111110011111101111000;    //HOOT
    }
}

void reverseArray(int arr[], int start, int end) { 
    int temp; 
    while (start < end) 
    { 
        temp = arr[start];    
        arr[start] = arr[end]; 
        arr[end] = temp; 
        start++; 
        end--; 
    }    
}  


void level1(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex) {

        //code for hexes
        const int hexgoal = 1;
        const int hexfailed = 2;
        const int hexshoot = 3;

        //constants
        const int headtoballx = 10;
        const int headtobally = 34;
        const int goalpostleftx = 98;
        const int goalposty = 0;
        const int goalpostwidth = 125;

        //allies
        *allynum = 2;
        //starting player
        ally[0].x = 50;
        ally[0].y = 200;
        ally[0].dir_x = 0;
        ally[0].dir_y = 0;
        //teammates
        ally[1].x = 200;
        ally[1].y = 100;
        ally[1].dir_x = 0;
        ally[1].dir_y = 1;
        ally[1].vel_y = 5;
        ally[1].y_min = 100;
        ally[1].y_max = 150;

        *foenum = 6;
        //wall
        foe[0].x = 70;
        foe[0].y = 100;
        foe[0].dir_x = 0;
        foe[0].dir_y = 0;
        foe[1].x = 90;
        foe[1].y = 100;
        foe[1].dir_x = 0;
        foe[1].dir_y = 0;
        foe[2].x = 110;
        foe[2].y = 100;
        foe[2].dir_x = 0;
        foe[2].dir_y = 0;
        foe[3].x = 130;
        foe[3].y = 100;
        foe[3].dir_x = 0;
        foe[3].dir_y = 0;
        foe[4].x = 150;
        foe[4].y = 100;
        foe[4].dir_x = 0;
        foe[4].dir_y = 0;
        //goalkeep
        foe[5].x = 100;
        foe[5].y = 10;
        foe[5].dir_x = 1;
        foe[5].x_max = 223;
        foe[5].x_min = 98;
        foe[5].vel_x = 20;
        foe[5].dir_y = 0;

        //set initial ball pos
        *curBallPosX = ally[0].x+headtoballx;
        *curBallPosY = ally[0].y+headtobally;

        write_hex(hexshoot);
        //set params
        *ignoreAlly = 0;
        *shoot = false;
        *angle = 20;
        *balltrajindex = 0;
    }


void level2(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex) {

        //code for hexes
        const int hexgoal = 1;
        const int hexfailed = 2;
        const int hexshoot = 3;

        //constants
        const int headtoballx = 10;
        const int headtobally = 34;
        const int goalpostleftx = 98;
        const int goalposty = 0;
        const int goalpostwidth = 125;

        //allies
        *allynum = 3;
        //starting player
        ally[0].x = 269;
        ally[0].y = 188;
        ally[0].dir_x = 0;
        ally[0].dir_y = 0;
        //teammates
        ally[1].x = 49;
        ally[1].y = 160;
        ally[1].dir_x = 0;
        ally[1].dir_y = 0;

        ally[2].x = 175;
        ally[2].y = 84;
        ally[2].dir_x = 1;
        ally[2].dir_y = 0;
        ally[2].vel_x = 5;
        ally[2].x_min = 170;
        ally[2].x_max = 282;


        *foenum = 7;
        //wall
        foe[0].x = 40;
        foe[0].y = 98;
        foe[0].dir_x = 0;
        foe[0].dir_y = 0;
        foe[1].x = 61;
        foe[1].y = 98;
        foe[1].dir_x = 0;
        foe[1].dir_y = 0;
        foe[2].x = 84;
        foe[2].y = 98;
        foe[2].dir_x = 0;
        foe[2].dir_y = 0;
        foe[3].x = 200;
        foe[3].y = 130;
        foe[3].dir_x = 0;
        foe[3].dir_y = 0;
        foe[4].x = 221;
        foe[4].y = 130;
        foe[4].dir_x = 0;
        foe[4].dir_y = 0;
        foe[5].x = 243;
        foe[5].y = 130;
        foe[5].dir_x = 0;
        foe[5].dir_y = 0;

        //goalkeep
        foe[6].x = 100;
        foe[6].y = 10;
        foe[6].dir_x = 1;
        foe[6].x_max = 223;
        foe[6].x_min = 98;
        foe[6].vel_x = 20;
        foe[6].dir_y = 0;

        //set initial ball pos
        *curBallPosX = ally[0].x+headtoballx;
        *curBallPosY = ally[0].y+headtobally;

        write_hex(hexshoot);
        //set params
        *ignoreAlly = 0;
        *shoot = false;
        *angle = 20;
        *balltrajindex = 0;
    }

void level3(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex) {

        //code for hexes
        const int hexgoal = 1;
        const int hexfailed = 2;
        const int hexshoot = 3;

        //constants
        const int headtoballx = 10;
        const int headtobally = 34;
        const int goalpostleftx = 98;
        const int goalposty = 0;
        const int goalpostwidth = 125;

        //allies
        *allynum = 3;
        //starting player
        ally[0].x = 286;
        ally[0].y = 159;
        ally[0].dir_x = 0;
        ally[0].dir_y = 0;
        //teammates
        ally[1].x = 46;
        ally[1].y = 80;
        ally[1].dir_x = 0;
        ally[1].dir_y = 0;

        ally[2].x = 220;
        ally[2].y = 182;
        ally[2].dir_x = 1;
        ally[2].dir_y = 0;
        ally[2].vel_x = 5;
        ally[2].x_min = 187;
        ally[2].x_max = 276;


        *foenum = 7;
        //wall
        foe[0].x = 250;
        foe[0].y = 32;
        foe[0].dir_x = 0;
        foe[0].dir_y = 0;
        foe[1].x = 250;
        foe[1].y = 76;
        foe[1].dir_x = 0;
        foe[1].dir_y = 0;
        foe[2].x = 250;
        foe[2].y = 121;
        foe[2].dir_x = 0;
        foe[2].dir_y = 0;
        foe[3].x = 200;
        foe[3].y = 130;
        foe[3].dir_x = 0;
        foe[3].dir_y = 0;
        foe[4].x = 139;
        foe[4].y = 106;
        foe[4].dir_x = 0;
        foe[4].dir_y = 0;
        foe[5].x = 159;
        foe[5].y = 106;
        foe[5].dir_x = 183;
        foe[5].dir_y = 106;

        //goalkeep
        foe[6].x = 100;
        foe[6].y = 10;
        foe[6].dir_x = 1;
        foe[6].x_max = 223;
        foe[6].x_min = 98;
        foe[6].vel_x = 20;
        foe[6].dir_y = 0;

        //set initial ball pos
        *curBallPosX = ally[0].x+headtoballx;
        *curBallPosY = ally[0].y+headtobally;

        write_hex(hexshoot);
        //set params
        *ignoreAlly = 0;
        *shoot = false;
        *angle = 20;
        *balltrajindex = 0;
    }

void level4(int* allynum, player ally[20], int* foenum, player foe[20], int* curBallPosX, int* curBallPosY,
    int* ignoreAlly, bool* shoot, int* angle, int* balltrajindex) {

        //code for hexes
        const int hexgoal = 1;
        const int hexfailed = 2;
        const int hexshoot = 3;

        //constants
        const int headtoballx = 10;
        const int headtobally = 34;
        const int goalpostleftx = 98;
        const int goalposty = 0;
        const int goalpostwidth = 125;

        //allies
        *allynum = 3;
        //starting player
        ally[0].x = 156;
        ally[0].y = 180;
        ally[0].dir_x = 0;
        ally[0].dir_y = 0;
        //teammates
        ally[1].x = 261;
        ally[1].y = 53;
        ally[1].dir_x = 0;
        ally[1].dir_y = 0;

        ally[2].x = 99;
        ally[2].y = 56;
        ally[2].dir_x = 0;
        ally[2].dir_y = 0;


        *foenum = 4;
        //wall
        foe[0].x = 156;
        foe[0].y = 118;
        foe[0].dir_x = 1;
        foe[0].dir_y = 0;
        foe[0].x_max = foe[0].x+60;
        foe[0].x_min = foe[0].x-40;
        foe[0].vel_x = 10;
        foe[1].x = 177;
        foe[1].y = 118;
        foe[1].dir_x = 1;
        foe[1].dir_y = 0;
        foe[1].x_max = foe[1].x+60;
        foe[1].x_min = foe[1].x-40;
        foe[1].vel_x = 10;
        foe[2].x = 201;
        foe[2].y = 118;
        foe[2].dir_x = 1;
        foe[2].dir_y = 0;
        foe[2].x_max = foe[2].x+60;
        foe[2].x_min = foe[2].x-40;
        foe[2].vel_x = 10;

        //goalkeep
        foe[3].x = 100;
        foe[3].y = 10;
        foe[3].dir_x = 1;
        foe[3].x_max = 223;
        foe[3].x_min = 98;
        foe[3].vel_x = 20;
        foe[3].dir_y = 0;

        //set initial ball pos
        *curBallPosX = ally[0].x+headtoballx;
        *curBallPosY = ally[0].y+headtobally;

        write_hex(hexshoot);
        //set params
        *ignoreAlly = 0;
        *shoot = false;
        *angle = 20;
        *balltrajindex = 0;
    }


