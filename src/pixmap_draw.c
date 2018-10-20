#include "pixmap_draw.h"

int pixmap_image_draw_line(PixMapImage *image, RGB *rgb, int x1, int y1, int x2, int y2)
{
    /* Make sure the line is drawn ascending in x so the loop doesn't go
       on forever */
    if(x1 > x2)
    {
        int temp = x1;
        x1 = x2;
        x2 = temp;

        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    int x = x1;
    double y = y1;
    double slope = (double) (y2 - y1) / (x2 - x1);

    int status = 0;

    for(; x != x2; x += 1, y += slope)
	{
	    pixmap_image_set_pixel_by_rgb(image, x, round(y), rgb, &status);

	    if(status)
		    return -1;
	}
    /* Set the last pixel */
    pixmap_image_set_pixel_by_rgb(image, x2, y2, rgb, &status);

    if(status)
	    return -1;

    return 0;
}

int pixmap_image_draw_rectangle_by_points(PixMapImage *image, RGB *rgb,
				    int x1, int y1, int x2, int y2)
{
    /* Start from lowest coordinates */
    if(x1 > x2)
	{
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }

    if(y1 > y2)
	{
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    /* Draw horizontal lines */
    int status = 0;
    for(int x = x1; x <= x2; x++)
	{
	    pixmap_image_set_pixel_by_rgb(image, x, y1, rgb, &status);

	    if(status)
	       return -1;

	    pixmap_image_set_pixel_by_rgb(image, x, y2, rgb, &status);

	    if(status)
		    return -1;
	}

    /* Draw vertical lines */
    for(int y = y1; y <= y2; y++)
	{
	    pixmap_image_set_pixel_by_rgb(image, x1, y, rgb, &status);

	    if(status)
		    return -1;

	    pixmap_image_set_pixel_by_rgb(image, x2, y, rgb, &status);

	    if(status)
		    return -1;
	}

    return 0;
}

int pixmap_image_draw_rectangle_by_size(PixMapImage *image, RGB *rgb, int x, int y,
				                    int dx, int dy)
{
    int x2 = x + dx;
    int y2 = y + dy;

    return pixmap_image_draw_rectangle_by_points(image, rgb, x, y, x2, y2);
}

int pixmap_image_draw_triangle(PixMapImage *image, RGB *rgb, int x1, int y1,
			       int x2, int y2, int x3, int y3)
{
    /* This will end up drawing the three vertex pixels twice each,
       but it's a small amount of overhead */
    int status = pixmap_image_draw_line(image, rgb, x1, x2, y1, y2);

    if(status)
	    return -1;
    status = pixmap_image_draw_line(image, rgb, x2, y2, x3, y3);
    
    if(status)
	    return -1;
    
    status - pixmap_image_draw_line(image, rgb, x3, y3, x1, y1);
    
    if(status)
	    return -1;
    
    return 0;
}
