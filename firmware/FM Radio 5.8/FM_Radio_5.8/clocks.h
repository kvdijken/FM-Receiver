#ifndef CLOCKS_H
#define CLOCKS_H

// clock identifiers
#define CLK_IF1 0
#define CLK_IF2 1

// drive level for the mixers.
#define NUM_DRIVE_LEVELS 5
#define MIN_DRIVELEVEL 0
#define MAX_DRIVELEVEL NUM_DRIVE_LEVELS-1
#define RADIX_DRIVELEVEL 1

// available drive levels
#define DRIVE_LEVEL_0MA 0
#define DRIVE_LEVEL_2MA 1
#define DRIVE_LEVEL_4MA 2
#define DRIVE_LEVEL_6MA 3
#define DRIVE_LEVEL_8MA 4

// default drive levels
#define DEFAULT_DRIVELEVEL_MIXER1 DRIVE_LEVEL_4MA
#define DEFAULT_DRIVELEVEL_MIXER2 DRIVE_LEVEL_2MA

#endif
