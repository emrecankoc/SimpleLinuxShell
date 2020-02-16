package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"os/signal"
	"regexp"
	"strings"
	"syscall"
)

// runs terminal commands
type CommandRunner struct {
	command string   // command to run
	args    []string // initial args
	stdIn   string   // input file
	stdOut  string   // output file
}

func cleanInputString(s string) (string, error) {
	resp := strings.TrimSpace(s)
	if len(resp) > 0 {
		return resp, nil
	}
	return "", errors.New("string is not valid")
}

func openFileWrapper(fileName string, isRead bool) (*os.File, error) {
	flag := os.O_RDONLY
	if !isRead {
		flag = os.O_CREATE | os.O_WRONLY
	}
	file, err := os.OpenFile(fileName, flag, 0644)
	if err != nil {
		fmt.Println("Error occured when opening file,", err.Error())
	}
	return file, err
}

func parseCommand(input string) (*CommandRunner, error) {
	// split input
	reg := regexp.MustCompile("\\s")
	indexes := reg.FindAllStringIndex(input, -1)

	cmdRunner := new(CommandRunner)
	cmdRunner.command = input[0:indexes[0][0]]
	cmdRunner.stdIn = ""
	cmdRunner.stdOut = ""

	var args []string
	// parse args
	startIndex := indexes[0][1]
	for i := 1; i < len(indexes); i++ {
		element := indexes[i]
		arg, err := cleanInputString(input[startIndex:element[0]])
		startIndex = element[1]
		if err != nil {
			continue
		} else if arg != ">" && arg != "<" {
			args = append(args, arg)
		} else if len(indexes) > i+1 {
			i++
			nextElement := indexes[i]
			nextArg, fErr := cleanInputString(input[element[1]:nextElement[0]])
			if fErr != nil {
				return nil, fErr
			}
			if arg == "<" && len(cmdRunner.stdIn) == 0 {
				cmdRunner.stdIn = nextArg
			} else if arg == ">" && len(cmdRunner.stdOut) == 0 {
				cmdRunner.stdOut = nextArg
			}
		}
	}
	cmdRunner.args = args

	return cmdRunner, nil
}

func (runner CommandRunner) execute() {
	if len(runner.command) > 0 {
		if runner.command == "quit" {
			beforeQuit()
		} else if runner.command == "cd" {
			chErr := os.Chdir(runner.args[1])
			if chErr != nil {
				fmt.Println(chErr.Error())
			}
		} else {
			cmd := exec.Command(runner.command, runner.args...)
			if len(runner.stdOut) == 0 {
				cmd.Stdout = os.Stdout
			} else {
				file, err := openFileWrapper(runner.stdOut, false)
				if err == nil {
					cmd.Stdout = file
				}
				defer file.Close()
			}
			if len(runner.stdIn) == 0 {
				cmd.Stdin = os.Stdin
			} else {
				file, err := openFileWrapper(runner.stdIn, false)
				if err == nil {
					cmd.Stdin = file
				}
				defer file.Close()
			}
			cmd.Stderr = os.Stderr
			err := cmd.Run()
			if err != nil {
				fmt.Println("Executing command failed:", err)
			}
		}
	}
}

func printCurrentPath() {
	currentPath, err := os.Getwd()
	if err != nil {
		panic("Path is not readable!")
	}
	fmt.Printf("%s >", currentPath)
}

func beforeQuit() {
	fmt.Println("\nStopping shell...")
	os.Exit(0)
}

func runShell() {
	fmt.Println("Starting shell...")

	sigs := make(chan os.Signal, 1)
	done := make(chan bool, 1)

	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)

	go func() {
		<-sigs
		beforeQuit()
		done <- true
	}()

	reader := bufio.NewReader(os.Stdin)
	for true {
		printCurrentPath()
		input, err := reader.ReadString('\n')
		if err != nil {
			fmt.Println("Command is not readable!")
			continue
		}

		cmd, err := parseCommand(input)
		if err != nil {
			continue
		} else {
			cmd.execute()
		}
	}
}

func main() {
	runShell()
}
