# SOFTENG 370 - Assignment 1
By Matthew Jakeman (mjak923)

## Building
You can build the program by running the following script:
```
./build.sh
```

Alternatively, this gcc command can be used:
```
cc src/main.c src/input.c src/history.c src/builtin.c src/invocation.c src/process.c src/job.c src/helper.c -o ash
```

## Running
Run the shell as follows:
```
./ash
```

Optionally with file redirection:
```
./ash < test1
```
