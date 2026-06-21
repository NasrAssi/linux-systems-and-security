# Unix Process Shell

A command-line shell written in C that reimplements the core of how a real shell drives the operating system: it parses a command line, forks child processes, executes programs, and wires up pipes, redirection, and signals.

## Features
- **External commands** via `fork` + `execvp`, with the parent `wait`ing for foreground jobs.
- **Background execution** (`cmd &`) — the shell does not block on the child.
- **Pipelines** (`cmd1 | cmd2`) — two children connected with a `pipe` and `dup2`.
- **I/O redirection** — `< input` and `> output` using `freopen`.
- **Built-ins** — `cd` and `quit`.
- **Signal control of other processes:**
  - `halt <pid>`  → `SIGTSTP` (suspend)
  - `wakeup <pid>` → `SIGCONT` (resume)
  - `ice <pid>`   → `SIGINT` (interrupt)
- **Debug mode** (`-d`) prints each child's PID and command to `stderr`.

## Files
- `myshell.c` — the shell itself: process control, pipelines, redirection, signals.
- `LineParser.c/.h` — command-line tokenizer (arguments, redirection, pipe, `&`).
- `mypipeline.c` — a standalone demo of the `ls -ls | wc` pipeline with verbose parent/child tracing, showing the `pipe`/`dup2`/`fork` mechanics step by step.
- `looper.c` — a test program that catches `SIGINT`/`SIGTSTP`/`SIGCONT`, handy for exercising the shell's `ice`/`halt`/`wakeup` commands.

## Build & run
```bash
make
./myshell            # or ./myshell -d for debug output
```
Example session:
```
/home/user> ls -l | grep .c > out.txt
/home/user> sleep 100 &
/home/user> halt 12345
```

## Concepts
Process control (`fork`/`exec`/`waitpid`), pipes and `dup2`, file-descriptor redirection, and inter-process signalling with `kill`.
