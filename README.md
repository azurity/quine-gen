# a multi-lang quine generator

This project is a toy to generate a quine program in multi-lang.
The generator tool is also write in multi-lang.

## HOW TO USE

- First install the environment of the languages what you want to use.

- Write the list file to configuring what language will be included.

- Then select a tool that your like to running the generator. The generate step will dependent the tools in all languages which is included in the list, however the entrance is arbitrary.

for example
```sh
node quine.js build list codefile
```

- Finally the tool will generate the quine source file, and tell you how to try the quine by giving a line of shell script.

> You can use `help` to get the help info of these tools. All of them have the same options.

## current support language & environment

| language      | environment   | tool file     |
|---------------|---------------|---------------|
| javascript    | nodejs        | quine.js      |
| python        | python3       | quine.py      |

