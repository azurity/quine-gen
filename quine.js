const process = require('process');
const fs = require('fs');
const child_process = require('child_process');

function help() {
    console.log(fs.readFileSync('help.txt', { encoding: 'utf-8' }));
}

function render(template, details) {
    let result = '';
    let i = 0;
    while (i < template.length) {
        let c = template[i];
        if (c == '$') {
            let j = 1;
            while (template[i + j] != '$') {
                j += 1;
            }
            let item = details.get(template.slice(i + 1, i + j));
            result += '"' + item[1] + details.get(item[3])[2] + '"';
            i += j;
        } else {
            result += c;
        }
        i += 1;
    }
    return result;
}

async function build() {
    let filename = process.argv[3];
    let langs = fs
        .readFileSync(filename, { encoding: 'utf-8' })
        .split('\n')
        .filter(l => l != '')
        .map(l => l.split('#'));
    let details = await Promise.all(langs.map(async lang =>
        [lang[0], [await new Promise((resolve,) => {
            child_process.exec(lang[1] + ' template ' + filename, (err, stdout) => {
                resolve(stdout.trim());
            });
        })]]
    ));
    for (let i = 0; i < langs.length; i++) {
        let exec_str = await new Promise((resolve,) => {
            let proc = child_process.exec(langs[i][1] + ' exec', (err, stdout) => {
                resolve(stdout);
            });
            proc.stdin.write(details[(i + 1) % langs.length][1][0] + '\n');
            proc.stdin.end();
        });
        let exec_strs = exec_str.split('\n');
        details[i][1].push(exec_strs[0]);
        details[i][1].push(exec_strs.slice(1).join('\n').trim());
        details[i][1].push(details[(i + 1) % langs.length][0]);
    }
    let detail_dict = new Map(details);
    let code = render(details[0][1][0], detail_dict) + details[0][1][2];
    let codefilename = process.argv[4];
    fs.writeFileSync(codefilename, code + '\n');
    let bashlist = ['cat ' + codefilename];
    for (let lang of langs) {
        bashlist.push(lang[2]);
    }
    bashlist.push(`diff ${codefilename} -`);
    console.log('try run:', bashlist.join('|'));
}

function main() {
    switch (process.argv[2]) {
        case 'template':
            let temp = fs
                .readFileSync(process.argv[3], { encoding: 'utf-8' })
                .split('\n')
                .filter(l => l != '')
                .map(l => l.split('#')[0])
                .map(lang => `${lang}=$${lang}$;`).join('');
            temp += '_=String.fromCharCode(34,10);__=JSON.stringify;';
            console.log(temp);
            break
        case 'exec':
            process.stdin.on('data', (data) => {
                let temp = data.toString('utf-8').trim();
                let exec_str = '';
                let args = [];
                let i = 0;
                while (i < temp.length) {
                    let c = temp[i];
                    if (c == '"') {
                        exec_str += '%s';
                        args.push('_[0]');
                    } else if (c == '\n') {
                        exec_str += '%s';
                        args.push('_[1]');
                    } else if (c == '$') {
                        j = 1
                        while (temp[i + j] != '$') {
                            j += 1;
                        }
                        exec_str += '%s';
                        args.push('__(' + temp.slice(i + 1, i + j) + ')');
                        i += j;
                    } else {
                        exec_str += c;
                    }
                    i += 1;
                }
                let final_part = `console.log(javascript,${args.join(',')})`;
                console.log(exec_str);
                console.log(final_part);
            });
            process.stdin.read();
            break
        case 'build':
            build()
            break
        case 'help':
            help()
            break
        default:
            console.log('unkown opntions, use "help" to get help.')
            help()
    }
}

main()
