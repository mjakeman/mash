# build.sh
# Matthew Jakeman (mjak923)

cc src/main.c src/input.c src/history.c src/builtin.c src/invocation.c src/process.c src/job.c src/helper.c -o ash
