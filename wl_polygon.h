#include "version.h"
#if defined(LWLIB) && defined(WOLFRAD) && defined(USE_POLYGON)

/* Describes a single point (used for a single vertex) */
struct PolyPoint {
    int X;   /* X coordinate */
    int Y;   /* Y coordinate */
};

/* Describes a series of points (used to store a list of vertices that
   describe a polygon; each vertex is assumed to connect to the two
   adjacent vertices, and the last vertex is assumed to connect to the
   first) */
struct PointListHeader {
    int Length;                /* # of points */
    struct PolyPoint* PointPtr;   /* pointer to list of points */
};

/* Describes the beginning and ending X coordinates of a single
   horizontal line */
struct HLine {
    int XStart; /* X coordinate of leftmost pixel in line */
    int XEnd;   /* X coordinate of rightmost pixel in line */
};

/* Describes a Length-long series of horizontal lines, all assumed to
   be on contiguous scan lines starting at YStart and proceeding
   downward (used to describe a scan-converted polygon to the
   low-level hardware-dependent drawing code) */
struct HLineList {
    int Length;                /* # of horizontal lines */
    int YStart;                /* Y coordinate of topmost line */
    struct HLine* HLinePtr;   /* pointer to list of horz lines */
};

extern int FillConvexPolygon(struct PointListHeader* VertexList,
    int Color, int XOffset, int YOffset, byte Blend);

extern int FillConvexPolygonEx(int Length,
    struct PolyPoint* PointPtr, int Color, byte Blend);
#endif