/*
 * Tommy MacWilliam, 2009
 * Malden Catholic High School Robotics
 *
 * Resuelve is licensed under the 
 * Creative Commons Attribution-Share Alike 3.0 United States.
 * For more information, see http://creativecommons.org/licenses/by-sa/3.0/us/
 *
 */

#include "stdio.h"
#include "stdlib.h" 
#include "string.h"
#include "unistd.h"
#include "math.h"

#include "resuelve_create.h"

void resuelve(struct ResuelveCourse *course, struct ResuelveSolver *solver, 
				char* filename)
{
	// mount usb
	resuelve_mount_usb ();
	// connect to create
	create_connect ();
	
	// create 2d array for map
	int **map;
	
	// save filename
	course->filename = filename;
	// don't animate path by default
	solver->animate_path = 0;
	// show path by default
	solver->show_path = 1;
	// default angle
	solver->angle = 0;
	// default drive speed
	solver->drive_speed = 500;
	// default turn speed
	solver->turn_speed = 300;
	// default block size in cm
	solver->block_size = 1.0;
	
	// get course size
	int course_size[2];
	resuelve_get_course_size (course, course_size);
	
	// allocate memory for map
	map = malloc ((course_size[0]) * sizeof (int*));
	int i = 0;
	for (i = 0; i < course_size[0]; i++)
	{
		map[i] = malloc ((course_size[1]) * sizeof (int));
	}
	
	// save course size and map to course struct
	course->size_x = course_size[0] - 1;
	course->size_y = course_size[1] - 1;
	course->map = map;
	
	// load course
	resuelve_load_course (course);	
}

/* mount a flash drive plugged into the CBC so it can be read
 * and written to
 */
void resuelve_mount_usb () 
{
    system ("chmod 777 /dev/sdb1");
    system ("mount -t vfat -o rw /dev/sdb1 /mnt/usercode");
	printf ("USB Mounted\n");
    msleep (1000);
}

/* unmount a flash drive plugged into the CBC so a program
 * can compile to the /mnt/usercode directory
 */
void resuelve_unmount_usb () 
{
    system ("umount /dev/sdb1");
	printf ("USB Unmounted\n");
    msleep (1000);
}

/* drive create given distance in cm at given wheel speeds
 */
void resuelve_create_drive (int speed, int dist) 
{
    int back = 0;
    float sleep_time;
    
    // convert from 0-1000 scale
    speed /= 2;
    // calculate time to sleep, given speed is in mm/sec
    sleep_time = (10.0 * (float) dist) / ((float) speed);
    
    // if negative distance is given, make time positive and speed negative
    if(dist < 0) {
        sleep_time = -sleep_time;
        speed = -speed;
    }
    
    // drive given distance
    create_drive_direct(speed, speed);
    sleep(sleep_time);
    create_stop();
}

/* turn a number of degrees at a given speed
 */
void resuelve_create_turn (int speed, int degrees) 
{
	// convert from 0-1000 scale
	speed /= 2;
    create_spin_block(speed, degrees);
}

/* get the size of given course
 * returns an array where the first element is the x size and the
 * second element is the y size
 */
void resuelve_get_course_size (struct ResuelveCourse *course, int *course_size)
{	
	// open specified file for reading
	FILE* file = fopen (course->filename, "r");
	char buffer[1000];
	
	// count the number of lines in the file
	int linecount = 1;
	while (fgets (buffer, sizeof buffer, file) != NULL)
	{
		linecount++;
	}
	// save x and y information
	course_size[0] = strlen (buffer);
	course_size[1] = linecount;
	
	// close file
	fclose (file);	
}

/* load maze into given course struct
 */
void resuelve_load_course (struct ResuelveCourse *course)
{
	// open file for reading
	FILE* file = fopen (course->filename, "r");
	char buffer[1000]; 
	
	// read file
	int y = 0;
	while (fgets (buffer, sizeof buffer, file) != NULL)
	{
		int x = 0;
		// read each character and save value in array
		for (x = 0; x < course->size_x; x++)
		{
			char contents[1000];
			sprintf (contents, "%c", buffer[x]);
			
			// save course layout in map array
			if (strcmp (contents, WALL_MARKER) == 0)
			{
				course->map[x][y] = WALL;
			}
			else if (strcmp (contents, OPEN_MARKER) == 0)
			{
				course->map[x][y] = OPEN;
			}
			else if (strcmp (contents, START_MARKER) == 0)
			{
				course->map[x][y] = START;
				course->start_x = x;
				course->start_y = y;
			}
			else if (strcmp (contents, FINISH_MARKER) == 0)
			{
				course->map[x][y] = FINISH;
				course->finish_x = x;
				course->finish_y = y;
			}
		}
		y++;
	}
	
	// close file
	fclose (file);
	printf("Course Loaded\n\n");
}

/* output representation of maze given by 2d array
 */
void resuelve_display_course (struct ResuelveCourse *course)
{
	int y, x;
	
	// iterate through rows
	for (y = 0; y < course->size_y; y++)
	{
		// iterate through columns
		for (x = 0; x < course->size_x; x++)
		{
			// display appropriate character from course map array
			if (course->map[x][y] == WALL)
			{
				printf (WALL_MARKER);
			}
			else if (course->map[x][y] == OPEN)
			{
				printf (OPEN_MARKER);
			}
			else if (course->map[x][y] == START)
			{
				printf (START_MARKER);
			}
			else if (course->map[x][y] == FINISH)
			{
				printf (FINISH_MARKER);
			}
			else if (course->map[x][y] == PATH)
			{
				printf (PATH_MARKER);
			}
			else if (course->map[x][y] == VISITED)
			{
				printf (VISITED_MARKER);
			}
		}
		printf ("\n");
	}
}

/* return 1 if solver has reached finish, 0 if not
 */
int resuelve_is_finish (struct ResuelveCourse *course, 
						struct ResuelveSolver *solver)
{
	// check if x and y coordinates match
	return (solver->x == course->finish_x && solver->y == course->finish_y);
}

/* calculate and display a path from start to finish
 */
void resuelve_calculate_path (struct ResuelveCourse *course, 
								struct ResuelveSolver *solver)
{
	int y, x;
	// iterate through rows
	for (y = 0; y < course->size_y; y++)
	{
		// iterate through columns
		for (x = 0; x < course->size_x; x++)
		{
			// save start coordinates
			if (course->map[x][y] == START)
			{
				solver->y = y;
				solver->x = x;
				course->map[x][y] = PATH;
			}
		}
	}
	
	// display maze and start/finish information
	resuelve_display_course (course);
	printf ("Start: %d, %d\n", solver->x, solver->y);
	printf ("Finish: %d, %d\n\n", course->finish_x, course->finish_y);
	
	// move through maze until finish is found
	while(solver->x != course->finish_x
			|| solver->y  != course->finish_y)
	{
		if (solver->animate_path)
		{
			sleep(1);
		}
		if (solver->show_path)
		{
			resuelve_display_course (course);
			printf("Current: %d, %d\n\n", solver->x, solver->y);
		}
		
		// blocked in on all four sides, so move away from finish and allow
		// visited spaces
		if (resuelve_check_obstacle (course, solver, UP)
					&& resuelve_check_obstacle (course, solver, RIGHT)
					&& resuelve_check_obstacle (course, solver, LEFT)
					&& resuelve_check_obstacle (course, solver, DOWN))
		{
			// need to go right, so try to go right
			if (solver->x < course->finish_x 
				&& !resuelve_check_wall (course, solver, RIGHT))
			{
				resuelve_move (course, solver, RIGHT);
				if (resuelve_is_finish (course, solver)) 
				{
					break; 
				}
				continue;
			}
			// need to go left, so try to go left
			else if (solver->x > course->finish_x
						&& !resuelve_check_wall (course, solver, LEFT))
			{
				resuelve_move (course, solver, LEFT);
				if (resuelve_is_finish (course, solver)) 
				{
					break; 
				}
				continue;
			}
			// need to go down, so try to go down
			else if (solver->y < course->finish_y
						&& !resuelve_check_wall (course, solver, DOWN))
			{
				resuelve_move (course, solver, DOWN);
				if (resuelve_is_finish (course, solver)) 
				{
					break;
				}
				continue;
			}
			// need to go up, so try to go up
			else if (solver->y > course->finish_y
						&& !resuelve_check_wall (course, solver, UP))
			{
				resuelve_move (course, solver, UP);
				if (resuelve_is_finish (course, solver)) 
				{
					break; 
				}
				continue;
			}
		}
		
		// need to go right, so try to go right
		if (solver->x < course->finish_x 
			&& !resuelve_check_obstacle (course, solver, RIGHT))
		{
			resuelve_move (course, solver, RIGHT);
			if (resuelve_is_finish (course, solver)) 
			{
				break; 
			}
		}
		// need to go left, so try to go left
		else if (solver->x > course->finish_x
					&& !resuelve_check_obstacle (course, solver, LEFT))
		{
			resuelve_move (course, solver, LEFT);
			if (resuelve_is_finish (course, solver)) 
			{
				break; 
			}
		}
		// need to go down, so try to go down
		else if (solver->y < course->finish_y
					&& !resuelve_check_obstacle (course, solver, DOWN))
		{
			resuelve_move (course, solver, DOWN);
			if (resuelve_is_finish (course, solver)) 
			{
				break; 
			}
		}
		// need to go up, so try to go up
		else if (solver->y > course->finish_y
					&& !resuelve_check_obstacle (course, solver, UP))
		{
			resuelve_move (course, solver, UP);
			if (resuelve_is_finish (course, solver)) 
			{
				break; 
			}
		}
				
		// can only go left
		if (resuelve_check_obstacle (course, solver, UP)
				&& resuelve_check_obstacle (course, solver, DOWN)
				&& resuelve_check_obstacle (course, solver, RIGHT))
		{
			resuelve_move (course, solver, LEFT);
			continue;
		}
		// can only go right
		else if (resuelve_check_obstacle (course, solver, UP)
					&& resuelve_check_obstacle (course, solver, DOWN)
					&& resuelve_check_obstacle (course, solver, LEFT))
		{
			resuelve_move (course, solver, RIGHT);
			continue;
		}
		// can only go up
		else if (resuelve_check_obstacle (course, solver, RIGHT)
					&& resuelve_check_obstacle (course, solver, DOWN)
					&& resuelve_check_obstacle (course, solver, LEFT))
		{
			resuelve_move (course, solver, UP);
			continue;
		}
		// can only go down
		else if (resuelve_check_obstacle (course, solver, UP)
					&& resuelve_check_obstacle (course, solver, RIGHT)
					&& resuelve_check_obstacle (course, solver, LEFT))
		{
			resuelve_move (course, solver, DOWN);
			continue;
		}
		// can go left or right
		else if (resuelve_check_obstacle (course, solver, UP)
					&& resuelve_check_obstacle (course, solver, DOWN))
		{
			if (solver->x < course->finish_x 
				&& !resuelve_check_obstacle (course, solver, RIGHT))
			{
				resuelve_move (course, solver, RIGHT);
				continue;
			}
			else if (!resuelve_check_obstacle (course, solver, LEFT))
			{
				resuelve_move (course, solver, LEFT);
				continue;
			}
		}
		// can only go up or down
		else if (resuelve_check_obstacle (course, solver, LEFT)
					&& resuelve_check_obstacle (course, solver, RIGHT))
		{
			if (solver->y < course->finish_y
				&& !resuelve_check_obstacle (course, solver, DOWN))
			{
				resuelve_move (course, solver, DOWN);
				continue;
			}
			else if (!resuelve_check_obstacle (course, solver, UP))
			{
				resuelve_move (course, solver, UP);
				continue;
			}
		}
		// can only go left or down
		else if (resuelve_check_obstacle (course, solver, RIGHT)
					&& resuelve_check_obstacle (course, solver, UP))
		{
			// farther away in the y, so go left
			if (abs (solver->x - course->finish_x < solver->y - course->finish_y)
				&& !resuelve_check_obstacle (course, solver, LEFT))
			{
				resuelve_move (course, solver, LEFT);
				continue;
			}
			else if (!resuelve_check_obstacle (course, solver, DOWN))
			{
				resuelve_move (course, solver, DOWN);
				continue;
			}
		}
		// can only go left or up
		else if (resuelve_check_obstacle (course, solver, RIGHT)
					&& resuelve_check_obstacle (course, solver, DOWN))
		{
			// farther away in the y, so go left
			if (abs (solver->x - course->finish_x < solver->y - course->finish_y)
				&& !resuelve_check_obstacle (course, solver, LEFT))
			{
				resuelve_move (course, solver, LEFT);
				continue;
			}
			else if (!resuelve_check_obstacle (course, solver, UP))
			{
				resuelve_move (course, solver, UP);
				continue;
			}
		}
		// can only go right or down
		else if (resuelve_check_obstacle (course, solver, LEFT)
					&& resuelve_check_obstacle (course, solver, UP))
		{
			// farther away in the y, so go left
			if (abs (solver->x - course->finish_x < solver->y - course->finish_y)
				&& !resuelve_check_obstacle (course, solver, RIGHT))
			{
				resuelve_move (course, solver, RIGHT);
				continue;
			}
			else if (!resuelve_check_obstacle (course, solver, DOWN))
			{
				resuelve_move (course, solver, DOWN);
				continue;
			}
		}
		// can only go right or up
		else if (resuelve_check_obstacle (course, solver, LEFT)
					&& resuelve_check_obstacle (course, solver, DOWN))
		{
			// farther away in the y, so go left
			if (abs (solver->x - course->finish_x < solver->y - course->finish_y)
				&& !resuelve_check_obstacle (course, solver, RIGHT))
			{
				resuelve_move (course, solver, RIGHT);
				continue;
			}
			else if (!resuelve_check_obstacle (course, solver, UP))
			{
				resuelve_move (course, solver, UP);
				continue;
			}
		}
	}

	// display completed maze
	resuelve_display_course (course);
	printf("Done\n");
}

/* move solver 1 unit in given direction
 */
void resuelve_move (struct ResuelveCourse *course, 
					struct ResuelveSolver *solver, int direction)
{
	// mark previous location as visited
	course->map[solver->x][solver->y] = VISITED;
	
	// turn create to appropriate angle
	if (solver->angle != direction)
	{
		resuelve_create_turn (solver->turn_speed, direction - (solver->angle));
	}	
	
	// make sure we are contained in maze before moving
	if (direction == UP && solver->y > 0) 
	{
		// change appropriate solver coordinate
		solver->y--;
	}
	else if (direction == DOWN && solver->y < course->size_y)
	{
		solver->y++;
	}
	else if (direction == LEFT && solver->x > 0)
	{
		solver->x--;
	}
	else if (direction == RIGHT && solver->x < course->size_x)
	{
		solver->x++;		
	}
	
	// change open marker to path marker to record path
	course->map[solver->x][solver->y] = PATH;

	// update angle
	solver->angle = direction;
	
	// drive forward one block
	resuelve_create_drive (solver->drive_speed, solver->block_size);
}

/* check for obstacle in given direction from the current solver position
 * return 1 if obstacle is found, 0 if path is clear
 */
int resuelve_check_obstacle (struct ResuelveCourse *course, 
								struct ResuelveSolver *solver, int direction)
{
	if (direction == UP) 
	{
		// make sure we are contained in maze
		if (solver->y > 0)
		{
			// check for presence of a wall or a visited space
			return (course->map[solver->x][solver->y - 1] == WALL)
					|| (course->map[solver->x][solver->y - 1] == VISITED);
		}
	}
	else if (direction == DOWN)
	{
		if (solver->y < course->size_y)
		{
			return (course->map[solver->x][solver->y + 1] == WALL)
					|| (course->map[solver->x][solver->y + 1] == VISITED);
		}
	}
	else if (direction == LEFT)
	{
		if (solver->x > 0)
		{
			return (course->map[solver->x - 1][solver->y] == WALL)
					|| (course->map[solver->x - 1][solver->y] == VISITED);
		}
	}
	else if (direction == RIGHT)
	{
		if (solver->x < course->size_x)
		{
			return (course->map[solver->x + 1][solver->y] == WALL)
					|| (course->map[solver->x + 1][solver->y] == VISITED);
		}
	}
}

/* check for visited space in given direction from the current solver position
 * return 1 if visited space is found, 0 if path is clear
 */
int resuelve_check_visited (struct ResuelveCourse *course, 
							struct ResuelveSolver *solver, int direction)
{
	if (direction == UP) 
	{
		// make sure we are contained in maze
		if (solver->y > 0)
		{
			// return 1 if visited above current position, 0 if not
			return (course->map[solver->x][solver->y - 1] == VISITED);
		}
	}
	else if (direction == DOWN)
	{
		if (solver->y < course->size_y)
		{
			return (course->map[solver->x][solver->y + 1] == VISITED);
		}
	}
	else if (direction == LEFT)
	{
		if (solver->x > 0)
		{
			return (course->map[solver->x - 1][solver->y] == VISITED);
		}
	}
	else if (direction == RIGHT)
	{
		if (solver->x < course->size_x)
		{
			return (course->map[solver->x + 1][solver->y] == VISITED);
		}
	}
}

/* check for wall in given direction from the current solver position
 * return 1 if wall is found, 0 if path is clear
 */
int resuelve_check_wall (struct ResuelveCourse *course, 
							struct ResuelveSolver *solver, int direction)
{
	if (direction == UP) 
	{
		// make sure we are contained in maze
		if (solver->y > 0)
		{
			// return 1 if wall above current position, 0 if not
			return (course->map[solver->x][solver->y - 1] == WALL);
		}
	}
	else if (direction == DOWN)
	{
		if (solver->y < course->size_y)
		{
			return (course->map[solver->x][solver->y + 1] == WALL);
		}
	}
	else if (direction == LEFT)
	{
		if (solver->x > 0)
		{
			return (course->map[solver->x - 1][solver->y] == WALL);
		}
	}
	else if (direction == RIGHT)
	{
		if (solver->x < course->size_x)
		{
			return (course->map[solver->x + 1][solver->y] == WALL);
		}
	}
}


void resuelve_set_start (struct ResuelveCourse *course, int start_x, 
							int start_y)
{
	// replace any starts with open space
	int y, x;
	// iterate through rows
	for (y = 0; y < course->size_y; y++)
	{
		// iterate through columns
		for (x = 0; x < course->size_x; x++)
		{
			if (course->map[x][y] == START)
			{
				course->map[x][y] = OPEN;
			}
		}
	}
	
	// create new start
	course->map[start_x][start_y] = START;
	course->start_x = start_x;
	course->start_y = start_y;
}

void resuelve_set_finish (struct ResuelveCourse *course, int finish_x, 
							int finish_y)
{
	// replace any finishes with open space
	int y, x;
	// iterate through rows
	for (y = 0; y < course->size_y; y++)
	{
		// iterate through columns
		for (x = 0; x < course->size_x; x++)
		{
			if (course->map[x][y] == FINISH)
			{
				course->map[x][y] = OPEN;
			}
		}
	}
	
	// create new finish
	course->map[x][y] = FINISH;
	course->finish_x = finish_x;
	course->finish_y = finish_y;
}

/* set angle for robot
 */
void resuelve_set_angle (struct ResuelveSolver *solver, int angle)
{
	solver->angle = angle;
}

/* set size of grid block in cm
 */
void resuelve_set_block_size (struct ResuelveSolver *solver, float block_size)
{
	solver->block_size = block_size;
}

/* animate path for solver, pausing after every iteration
 * set path_animate to 1 to animate, 0 (default) if not
 */
void resuelve_set_animate_path (struct ResuelveSolver *solver, int animate)
{
	solver->animate_path = animate;
}

/* show map and path of create
 */
void resuelve_set_show_path (struct ResuelveSolver *solver, int show)
{
	solver->show_path = show;
}

/* set drive speed for create
 */
void resuelve_set_create_drive_speed (struct ResuelveSolver *solver, int speed)
{
	solver->drive_speed = speed;
}

/* set turn speed for create
 */
void resuelve_set_create_turn_speed (struct ResuelveSolver *solver, int speed)
{
	solver->turn_speed = speed;
}
