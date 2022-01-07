import sys
import subprocess


def help():
    file = open('help.txt')
    print(file.read())
    file.close()


def render(template, details):
    result = ''
    i = 0
    while i < len(template):
        c = template[i]
        if c == '$':
            j = 1
            while template[i+j] != '$':
                j += 1
            item = details[template[i+1:i+j]]
            result += '"' + item[1] + details[item[3]][2] + '"'
            i += j
        else:
            result += c
        i += 1
    return result


def build():
    langs = []
    details = []
    filename = sys.argv[2]
    file = open(filename)
    for line in file.readlines():
        if line == '':
            continue
        langs.append(line.strip().split('#'))
    for lang in langs:
        details.append([lang[0], subprocess.getoutput(
            lang[1] + ' template ' + filename)])
    for i in range(len(langs)):
        exec_str = subprocess.check_output(
            langs[i][1] + ' exec', shell=True, input=bytes(details[(i + 1) % len(langs)][1], encoding='utf-8'))
        exec_strs = str(exec_str, encoding='utf-8').split('\n')
        details[i].append(exec_strs[0])
        details[i].append('\n'.join(exec_strs[1:]).strip())
        details[i].append(details[(i + 1) % len(langs)][0])
    detail_dict = {}
    for d in details:
        detail_dict[d[0]] = d[1:]
    code = render(details[0][1], detail_dict) + details[0][3]
    codefilename = sys.argv[3]
    codefile = open(codefilename, 'w')
    codefile.write(code + '\n')
    codefile.close()
    bashlist = ['cat ' + codefilename]
    for lang in langs:
        bashlist.append(lang[2])
    bashlist.append(f'diff {codefilename} -')
    print('try run:', '|'.join(bashlist))


def main():
    if sys.argv[1] == 'template':
        file = open(sys.argv[2])
        temp = ''
        for line in file.readlines():
            if line == '':
                continue
            lang = line.split('#')[0]
            temp += f'{lang}=${lang}$;'
        print(temp)
    elif sys.argv[1] == 'exec':
        # temp = input()
        temp = ''
        for line in sys.stdin:
            temp += line + '\n'
        temp = temp.strip()
        exec_str = ''
        counter = 2
        names = []
        i = 0
        while i < len(temp):
            c = temp[i]
            if c == '"':
                exec_str += '{0}'
            elif c == '\n':
                exec_str += '{1}'
            elif c == '$':
                j = 1
                while temp[i+j] != '$':
                    j += 1
                names.append(temp[i+1:i+j])
                exec_str += f'{{0}}{{{counter}}}{{0}}'
                counter += 1
                i += j
            else:
                exec_str += c
            i += 1
        final_part = f'print(python.format(chr(34),chr(10),{",".join(names)}))'
        print(exec_str)
        print(final_part)
    elif sys.argv[1] == 'build':
        build()
    elif sys.argv[1] == 'help':
        help()
    else:
        print('unkown opntions, use "help" to get help.')
        help()


if __name__ == '__main__':
    main()
