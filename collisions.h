//RECTANGLE STRUCT AND ALL ASSOCIATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------------------------------------

#ifndef COLLISION_H
#define COLLISION_H

typedef struct Rectangle{
    
    int tLeftCorner[2]; //position of top left corner of rectangle stored as [x,y]
    int tRightCorner[2]; //position of top right corner of rectangle
    int bLeftCorner[2]; //position of bottom left corner of rectangle
    int bRightCorner[2]; //position of bottom right corner of rectangle

}Rectangle ;

//Creates a new rectangle struct
Rectangle* NewRectangle(int x, int y, int length, int height){
    Rectangle *rect = malloc(sizeof(Rectangle));

    rect->tLeftCorner[0] = x;//set up coords of rectangle corners
    rect->tLeftCorner[1] = y;

    rect->tRightCorner[0] = x+length;
    rect->tRightCorner[1] = y;

    rect->bLeftCorner[0] = x;
    rect->bLeftCorner[1] = y+(height-1);

    rect->bRightCorner[0] = x+length;
    rect->bRightCorner[1] = y+(height-1);

    return rect;
}

//Changes coords of the rectangle corners to coincide with the new position of the object when it moves
void ResetBounds(Rectangle *rect, int x, int y, int length, int height){
    rect->tLeftCorner[0] = x;
    rect->tLeftCorner[1] = y;

    rect->tRightCorner[0] = x+length;
    rect->tRightCorner[1] = y;

    rect->bLeftCorner[0] = x;
    rect->bLeftCorner[1] = y+(height-1);

    rect->bRightCorner[0] = x+length;
    rect->bRightCorner[1] = y+(height-1);

}

//Checks for all of the cases where the rectanges could not be intersecting to see whether they are intersecting
int DoIntersect(Rectangle *rect1, Rectangle *rect2){ //returns 0 for no and 1 for yes

    if((rect1->tLeftCorner[0] > rect2->tRightCorner[0]) || (rect1->bRightCorner[0] < rect2->tLeftCorner[0])) //if top left of rect1 is to the right of the bottom right of rect2 or vice versa  
    {
        return 0;
    }

    if((rect1->tLeftCorner[1] > rect2->bRightCorner[1]) || (rect1->bRightCorner[1] < rect2->tLeftCorner[1])) //if top left of rect1 is to the below of the bottom right of rect2 or vice versa  
    {
        return 0;
    }

    return 1;
}

//iterates through the four corner of a rectangle to see if they are passed any of the borders
int HasPassedBorder(Rectangle *rect, int borderminX, int borderminy, int bordermaxX, int bordermaxy){

    if(rect->bLeftCorner[0] <= borderminX || rect->bLeftCorner[1] >= bordermaxy){
        return 1;
    }

    if(rect->bRightCorner[0] >= bordermaxX || rect->bRightCorner[1] >= bordermaxy){
        return 1;
    }

     if(rect->tLeftCorner[0] <= borderminX || rect->tLeftCorner[1] <= borderminy){
        return 1;
    }

    if(rect->tRightCorner[0] >= bordermaxX || rect->tRightCorner[1] <= borderminy){
        return 1;
    }

    return 0; //the rectange has not passed the border 
}

//Physics/Maths functions like calculating velocity and clamping values

//thrust is thrust force, u is previous velocity, drag is the drag coefficient, mass if the mass of the ship
float CalculateVelocity(int thrust, float time, int u, float drag, int mass){
    float finalVelocity = (thrust*time + u*mass)/(mass+drag*time);
    return finalVelocity;
}

//ensures value doens't excede max and min values
float ClampValue(float value, float max, float min){
    if(value > max){
        value = max;
    }else if(value < min){
        value = min;
    }

    return value;
}

//ensures value doens't excede max and min values
int ClampInteger(int value, int max, int min){
    if(value > max){
        value = max;
    }else if(value < min){
        value = min;
    }

    return value;
}

//Generates a random number between two values
int GenerateRandomBetween(int min, int max){
     double val =  ((double)rand()/RAND_MAX) * (max - min) + min;
     return round(val);
}

#endif