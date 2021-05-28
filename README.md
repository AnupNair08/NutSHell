### NutSHell - A shell to help you fork. 

NutSHell is a UNIX based shell program that can be used to spawn and run processes by issuing commands. 
NutSHell uses the `fork()` and `exec()` system calls to do process creation and also supports full job control. Additionally IO redirection and piping are also supported.

#### Features

1. Running commands without the full path name.
2. Running jobs in the background.
3. Piping and IO redirection.
4. Prompt and current working directory status.
5. Signal Handling.
6. In built commands. 

#### To run NutSHell
- Run `make` command in the source directory of the project.
- It should compile the source code and then start running the shell.
- The source code can be found in the `src/` directory and the complied binaries in the `bin/` directory


#### References

- [Writing a Shell](https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf)
- [At a glance](https://condor.depaul.edu/glancast/374class/hw/shlab-readme.html)
- [Signals](https://cs.brown.edu/courses/csci0330/docs/labs/signals.pdf)
- [FAQs](https://www.andrew.cmu.edu/course/15-310/applications/homework/homework4/lab4-faq.pdf)


#### Contributing

Pull Requests are welcome