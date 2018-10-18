#include "pixmap.h"

struct _PixMapImage
{
    char *_file_name;
    PixMapImageType _type;
    int _width;
    int _height;
    RGB *_pixels;
    int _max_color_value;
};

static int get_metadata_value(FILE *file);

PixMapImage *pixmap_image_new(char const *name, int width, int height, int max_color_val, PixMapImageType type)
{
    PixMapImage *new_image = malloc(sizeof(*new_image));

    if(!new_image) return 0;

    new_image->_width = width;
    new_image->_height = height;
    new_image->_max_color_value = max_color_val <= 255 ? max_color_val : 255;

    new_image->_pixels = malloc(sizeof(RGB) * (new_image->_width * new_image->_height));

    if(!new_image->_pixels)
    {
        free(new_image);
        return 0;
    }

    new_image->_file_name = name;
    new_image->_type = type;

    return new_image;
}

PixMapImage *pixmap_image_open(char const *name)
{
    FILE *image_file = fopen(name, "r+");
    if(!image_file)
    {
        return 0;
    }

    PixMapImage *new_image = malloc(sizeof(*new_image));

    if(!new_image)
    {
        fclose(image_file);
        return 0;
    }
    
    new_image->_file_name = name;

    unsigned char sig[3];
    fread(sig, sizeof(unsigned char), 2, image_file);

    if(!(sig[0] == 'P' && sig[1] == '3'))
    {
        fclose(image_file);
        free(new_image);
        return 0;
    }

    new_image->_width = get_metadata_value(image_file);
    new_image->_height = get_metadata_value(image_file);
    new_image->_max_color_value = get_metadata_value(image_file);

    new_image->_width = new_image->_width > 0 ? new_image->_width : 1;
    new_image->_height = new_image->_height > 0 ? new_image->_height : 1;

    new_image->_pixels = malloc(sizeof(RGB) * (new_image->_width * new_image->_height));


    if(!new_image->_pixels)
    {
        fclose(image_file);
        free(new_image);
        return 0;
    }

    int c;
    RGB pixel;
    unsigned int value_in = 0;
    int rgb_counter = 1;
    int file_pos = 0;
    
    /* Read in the pixel values */
    while((c = fgetc(image_file)) != EOF)
    {
        value_in = 0;
        if(isdigit(c))
        {
            ungetc(c, image_file);
            if(rgb_counter > 3)
            {
                new_image->_pixels[file_pos] = pixel;
                file_pos++;
                rgb_counter = 1;
            }

            while((c = fgetc(image_file)) != EOF && c != ' ' && c != '\n')
            {
                value_in *= 10;
                value_in += (c - '0');
            }

            if(rgb_counter == 1)
                pixel.red = value_in;
            else if(rgb_counter == 2)
                pixel.green = value_in;
            else if(rgb_counter == 3)
                pixel.blue = value_in;
            
            rgb_counter++;
        }
    }

    fclose(image_file);

    return new_image;
}

PixMapImage *pixmap_image_copy(PixMapImage *image, char const *new_name, PixMapImageType type)
{
    PixMapImage *copy_image = pixmap_image_new(new_name, image->_width, image->_height, image->_max_color_value, type);

    if(!copy_image) return 0;

    int i = 0;
    while(i < copy_image->_width * copy_image->_height)
    {
        copy_image->_pixels[i] = image->_pixels[i];
        i++;
    }

    return copy_image;
}

RGB *pixmap_image_get_pixel_array(PixMapImage *image)
{
    return image->_pixels;
}

void pixmap_image_set_pixel(PixMapImage *image, int x, int y, int red, int green, int blue, int *error)
{
    if(x > (image->_width - 1) || y > (image->_height - 1)
       || x < 0 || y < 0 || red < 0 || green < 0 || blue < 0)
    {
        if(error) *error = 1;
        return;
    }

    red = red <= 255 ? red : 255;
    green = green <= 255 ? green : 255;
    blue = blue <= 255 ? blue : 255;

    RGB color = { red, green, blue };

    if(error) *error = 0;
    image->_pixels[x + (y * image->_width)] = color;
}

void pixmap_image_set_pixel_by_rgb(PixMapImage *image, int x, int y, RGB *rgb, int *error)
{
    pixmap_image_set_pixel(image, x, y, rgb->red, rgb->green, rgb->blue, error);
}

RGB pixmap_image_get_pixel(PixMapImage *image, int x, int y)
{
    if(x > (image->_width - 1) || y > (image->_height - 1)
       || x < 0 || y < 0)
    {
        RGB error_pixel = {-1, -1, -1};
        return error_pixel;
    }

    return image->_pixels[x + (y * image->_width)];
}

int pixmap_image_save(PixMapImage *image)
{
    FILE *image_file = fopen(image->_file_name, "w");

    if(!image_file)
        return -1;

    if(image->_type == Text)
    {
        fprintf(image_file, "%s\n%d %d\n%d\n", "P3", image->_width, image->_height, image->_max_color_value);

        int i = 0;
        int w = 0;
        while(i < image->_width * image->_height)
        {
            if(w > 2)
            {
                fprintf(image_file, "%d %d %d\n",
                        image->_pixels[i].red, image->_pixels[i].green, image->_pixels[i].blue);
                w = 0;
            } else {
                fprintf(image_file, "%d %d %d ",
                        image->_pixels[i].red, image->_pixels[i].green, image->_pixels[i].blue);

            }

            w++;
            i++;
        }
    } else if(image->_type == Binary)
    {
        unsigned char *temp_pixels = malloc(sizeof(unsigned char) * image->_width * image->_height * 3);
        
        if(!temp_pixels)
        {
            fclose(image_file);
            return -1;
        }
        
        for(int y = 0; y < image->_height; y++)
        {
            for(int x = 0; x < image->_width; x++)
            {
                temp_pixels[x + (y * image->_width)] = image->_pixels[x + (y * image->_width)].red;
                temp_pixels[(x + (y * image->_width) + 1)] = image->_pixels[x + (y * image->_width)].green;
                temp_pixels[(x + (y * image->_width) + 2)] = image->_pixels[x + (y * image->_width)].blue;
            }
        }

        fprintf(image_file, "%s\n%d %d\n%d\n", "P6", image->_width, image->_height, image->_max_color_value);
        fwrite(temp_pixels, sizeof(unsigned char), image->_width * image->_height, image_file);

        free(temp_pixels);
    }

    fclose(image_file);

    return 0;
}

int pixmap_image_get_width(PixMapImage *image)
{
    return image->_width;
}

int pixmap_image_get_height(PixMapImage *image)
{
    return image->_height;
}

int pixmap_image_get_max_color_value(PixMapImage *image)
{
    return image->_max_color_value;
}

void pixmap_image_foreach_pixel(PixMapImage *image,
				void (*func)(RGB pixel, void *func_arg), void *arg)
{
    int total_pixels = image->_width * image->_height;
    for(int i = 0; i < total_pixels; i++)
	    (*func)(image->_pixels[i], arg);
}

void pixmap_image_close(PixMapImage *image)
{
    if(!image) return;

    free(image->_pixels);
    free(image);
}

static int get_metadata_value(FILE *fin)
{
    int c, res = 0;
    int comment = 0;
    while((c = fgetc(fin)) != EOF)
    {
        if(c == '#') comment = 1;
        if(c == '\n') comment = 0;

        if(isdigit(c) && !comment)
        {
            ungetc(c, fin);
            fscanf(fin, "%d", &res);
            break;
        }
    }

    return res;
}
