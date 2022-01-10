package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"
)

func help() {
	data, _ := ioutil.ReadFile("help.txt")
	fmt.Println(string(data))
}

func render(template string, details map[string][]string) string {
	result := ""
	i := 0
	tempRune := []rune(template)
	for i < len(tempRune) {
		c := tempRune[i]
		if c == '$' {
			j := 1
			for tempRune[i+j] != '$' {
				j += 1
			}
			item := details[string(tempRune[i+1:i+j])]
			result += "\"" + item[1] + details[item[4]][3] + "\""
			i += j
		} else {
			result += string(c)
		}
		i += 1
	}
	return result
}

func build(filename string, codefilename string) {
	langs := [][]string{}
	details := [][]string{}
	buffer, _ := ioutil.ReadFile(os.Args[2])
	lines := strings.Split(string(buffer), "\n")
	for _, line := range lines {
		if line == "" {
			continue
		}
		langs = append(langs, strings.Split(line, "#"))
	}
	for _, lang := range langs {
		cmd := strings.Split(lang[1]+" template "+filename, " ")
		output, _ := exec.Command(cmd[0], cmd[1:]...).Output()
		temp := strings.TrimSpace(string(output))
		details = append(details, []string{lang[0], temp})
	}
	for i, _ := range langs {
		cmd := strings.Split(langs[i][1]+" exec", " ")
		proc := exec.Command(cmd[0], cmd[1:]...)
		stdin, _ := proc.StdinPipe()
		stdin.Write([]byte(details[(i+1)%len(langs)][1] + "\n"))
		stdin.Close()
		output, _ := proc.Output()
		lines = strings.Split(string(output), "\n")
		details[i] = append(details[i], lines[0])
		details[i] = append(details[i], strings.Join(lines[1:], "\n"))
		//
		antiCmd := strings.Split(langs[(i-1+len(langs))%len(langs)][1]+" anti-escape", " ")
		antiProc := exec.Command(antiCmd[0], antiCmd[1:]...)
		antiStdin, _ := antiProc.StdinPipe()
		antiStdin.Write([]byte(details[i][3] + "\n"))
		antiStdin.Close()
		antiOut, _ := antiProc.Output()
		details[i] = append(details[i], strings.TrimSpace(string(antiOut)))
		details[i] = append(details[i], details[(i+1)%len(langs)][0])
	}
	detailDict := make(map[string][]string)
	for _, d := range details {
		detailDict[d[0]] = d[1:]
	}
	code := render(details[0][1], detailDict) + details[0][3]
	ioutil.WriteFile(codefilename, []byte(code), 0755)
	fmt.Printf("try run: cat %s", codefilename)
	for _, lang := range langs {
		fmt.Print("|" + lang[2])
	}
	fmt.Printf("|diff %s -\n", codefilename)
}

func main() {
	if len(os.Args) < 2 {
		return
	}
	if os.Args[1] == "template" && len(os.Args) == 3 {
		buffer, _ := ioutil.ReadFile(os.Args[2])
		lines := strings.Split(string(buffer), "\n")
		fmt.Println("package main")
		fmt.Println("import \"fmt\"")
		fmt.Print("const(_qu=string(rune(34));_ln=string(rune(10))")
		for _, line := range lines {
			if line == "" {
				continue
			}
			name := strings.Split(line, "#")[0]
			fmt.Printf(";%s=$%s$", name, name)
		}
		fmt.Println(");")
	} else if os.Args[1] == "exec" {
		lines := []string{}
		scanner := bufio.NewScanner(os.Stdin)
		for scanner.Scan() {
			line := scanner.Text()
			lines = append(lines, line)
		}
		temp := []rune(strings.Join(lines, "\n"))
		// fmt.Println(lines)
		names := []string{}
		i := 0
		fmt.Print("")
		for i < len(temp) {
			c := temp[i]
			if c == '"' {
				fmt.Print("%s")
				names = append(names, "_qu")
			} else if c == '\n' {
				fmt.Print("%s")
				names = append(names, "_ln")
			} else if c == '$' {
				j := 1
				for temp[i+j] != '$' {
					j += 1
				}
				names = append(names, "_qu", string(temp[i+1:i+j]), "_qu")
				fmt.Print("%s%s%s")
				i += j
			} else {
				fmt.Print(string(c))
			}
			i += 1
		}
		fmt.Print("\n")
		fmt.Printf("func main(){fmt.Printf(golang,%s);fmt.Print(_ln)}\n", strings.Join(names, ","))
	} else if os.Args[1] == "anti-escape" {
		lines := []string{}
		scanner := bufio.NewScanner(os.Stdin)
		for scanner.Scan() {
			line := scanner.Text()
			lines = append(lines, line)
		}
		temp := strings.Join(lines, "\n")
		for _, c := range temp {
			if c == '%' {
				fmt.Print("%%")
			} else {
				fmt.Print(string(c))
			}
		}
	} else if os.Args[1] == "build" && len(os.Args) == 4 {
		build(os.Args[2], os.Args[3])
	} else if os.Args[1] == "help" {
		help()
	} else {
		fmt.Println("unkown opntions, use \"help\" to get help.")
		help()
	}
	return
}
