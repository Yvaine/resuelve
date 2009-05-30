/*
 * Tommy MacWilliam, 2009
 * Malden Catholic High School Robotics
 *
 * Resuelve is licensed under the 
 * Creative Commons Attribution-Share Alike 3.0 United States.
 * For more information, see http://creativecommons.org/licenses/by-sa/3.0/us/
 *
 */

#define WALL 0
#define OPEN 1
#define START 2
#define FINISH 3
#define PATH 4
#define VISITED 5

#define WALL_MARKER "x"
#define OPEN_MARKER "."
#define START_MARKER "s"
#define FINISH_MARKER "f"
#define PATH_MARKER "t"
#define VISITED_MARKER "o"

#define UP 88
#define RIGHT 352
#define DOWN 264
#define LEFT 176

#define RESUELVE_DEBUG 0

typedef int** RESUELVE_MAP;

struct ResuelveSolver
{
	int x;
	int y;
	int show_path;
	int animate_path;
	int angle;
	int drive_speed;
	int turn_speed;
	float block_size;
};

struct ResuelveCourse
{
	char* filename;
	int size_x;
	int size_y;
	int start_x;
	int start_y;
	int finish_x;
	int finish_y;
	int** map;
};

void resuelve (struct ResuelveCourse*, struct ResuelveSolver*, char*);
void resuelve_mount_usb ();
void resuelve_unmout_usb ();
void resuelve_create_drive (int, int);
void resuelve_create_turn (int, int);
void resuelve_get_course_size (struct ResuelveCourse*, int *course_size);
void resuelve_load_course (struct ResuelveCourse*);
void resuelve_display_course (struct ResuelveCourse*);
void resuelve_calculate_path (struct ResuelveCourse*, struct ResuelveSolver*);
int resuelve_check_obstacle (struct ResuelveCourse*, struct ResuelveSolver*, int);
int resuelve_check_visited (struct ResuelveCourse*, struct ResuelveSolver*, int);
int resuelve_check_wall (struct ResuelveCourse*, struct ResuelveSolver*, int);
void resuelve_move (struct ResuelveCourse*, struct ResuelveSolver*, int);
void resuelve_set_start (struct ResuelveCourse*, int, int);
void resuelve_set_finish (struct ResuelveCourse*, int, int);
void resuelve_set_angle (struct ResuelveSolver*, int);
void resuelve_set_block_size (struct ResuelveSolver*, float);
void resuelve_set_animate_path (struct ResuelveSolver*, int);
void resuelve_set_show_path (struct ResuelveSolver*, int);
void resuelve_set_create_drive_speed (struct ResuelveSolver*, int);
void resuelve_set_create_turn_speed (struct ResuelveSolver*, int);
