#ifndef TRAPEZOIDSEGMENT_H
#define TRAPEZOIDSEGMENT_H

#undef DEBUG_TRAPEZOIDSEGMENT

class TrapezoidSegment {
    private:
        int x1, y1; // Coordinates of the first corner
        int x2, y2; // Coordinates of the second corner
        int x3, y3; // Coordinates of the third corner
        int x4, y4; // Coordinates of the fourth corner

    public:
        TrapezoidSegment(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
        int getX1();
        int getY1();
        int getX2();
        int getY2();
        int getX3();
        int getY3();
        int getX4();
        int getY4();
};

#endif // TRAPEZOIDSEGMENT_H