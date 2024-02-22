#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <cJSON.h>

#define IW 800
#define IH 600
#define FZ 36

void draw_text_center(cairo_t *cr, const char *text, double x, double y) {
    cairo_text_extents_t extents;
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, x - extents.width / 2 - extents.x_bearing,
                  y - font_extents.descent - font_extents.height / 2);
    cairo_show_text(cr, text);
}

int main() {
    FILE *file = fopen("names.json", "r");
    if (!file) {
        fprintf(stderr, "ohh, There is an error to open JSON file.\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *json_data = (char *)malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    fclose(file);
    json_data[file_size] = '\0';

    cJSON *root = cJSON_Parse(json_data);
    free(json_data);
    if (!root) {
        fprintf(stderr, "ohh, There is an error to parse JSON file.\n");
        return 1;
    }
    system("mkdir -p certifications");
    cJSON *names = cJSON_GetObjectItem(root, "names");
    for (int i = 0; i < cJSON_GetArraySize(names); i++) {
        cJSON *name = cJSON_GetArrayItem(names, i)->valuestring;

        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IW, IH);
        cairo_t *cr = cairo_create(surface);

        cairo_surface_t *cert_image = cairo_image_surface_create_from_png("certification.png");
        cairo_set_source_surface(cr, cert_image, 0, 0);
        cairo_paint(cr);

        cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_FZ(cr, FZ);
        cairo_set_source_rgb(cr, 0, 0, 0);
        draw_text_center(cr, name->valuestring, IW / 2, IH / 2);

        char filename[50];
        sprintf(filename, "certification/%s.png", name->valuestring);
        cairo_surface_write_to_png(surface, filename);

        cairo_surface_destroy(surface);
        cairo_surface_destroy(cert_image);
        cairo_destroy(cr);
    }

    char cmd[100];
    sprintf(cmd, "cd certification && convert *.png certification.pdf && cd .. && rm -rf certification/*.png");
    system(cmd);

    cJSON_Delete(root);

    return 0;
}
