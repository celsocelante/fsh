# fsh
Descrição do Trabalho

Vocês devem implementar na linguagem C uma shell denominada fsh (family shell) para colocar
em prática os princípios de manipulação de processos.
Ao iniciar, fsh exibe seu prompt “fsh>”(os símbolos no começo de cada linha indicando que a
shell está à espera de comandos). Quando ela recebe uma linha de comando do usuário o
processamento da mesma começa. Primeiro, a linha deve ser interpretada em termos da linguagem
de comandos definida a seguir e cada comando identificado deve ser executado. Essa operação
possivelmente levará ao disparo de novos processos.

Um diferencial da fsh é que, ao contrário das shells UNIX convencionais, ao processar um
comando que seja um programa para ser executado ela deverá criar o processo para executar este
programa em background (a fsh retorna imediatamente para receber novos comandos).
Outro diferencial da fsh é que ela permite que em uma mesma linha de comandos o usuário solicite
a execução de vários programas em paralelo mas seguindo uma hierarquia da seguinte forma:

fsh> comando1 @ comando2 @ comando3 ...

Neste exemplo a shell deverá primeiramente, criar um processo “gerente” P0. Então, o processo P0,
deverá criar os processos P1 (para executar o comando1), P2 (para executar o comando2), P3 para
executar o comando3). Desta forma é criada uma família de processos em que o processo “gerente”
P0 é pai de P1, P2, P3,... até o processo referente ao último comando da linha (a lista suporta até 10
comandos). Lembre-se que tando P0, quanto os outros Px devem ser processos de background e,
portanto, não devem estar associados a nenhum Terminal.

Outra particularidade da fsh é que uma família de processos é muito unida! Primeiramente porque
todos os processos criados pelo processo “gerente” devem ignorar o sinal SIGTSTP (eles não
“gostam” de ser suspensos!!). Então, sempre que o usuário clicar “Ctrl-z”, o processo “gerente”
deverar exibir a mensagem: “Não adianta tentar suspender... minha família de processos está
protegida!”. Mas se o usuário clicar “Ctrl-z” e apenas a fsh estiver rodando, nada acontece.
Além disso, quando um dos processos morre, todos os demais processos da família morrem juntos,
à exceção da fsh (que é highlander). Esse genocídio de processos deve ser implementado com o
auxílio de sinais (alterando-se a rotina de tratamento de um determinado sinal).
Finalmente, a nossa fsh não quer saber de morte súbita enquanto ela tiver filhos ainda vivos...
(muito responsável!). Com isso ela deve BLOQUEAR (não é ignorar) o sinal gerado pelo Ctrl-C
enquanto ela tiver filhos. Quando ela não tiver nenhum filho vivo, ela pode ir descansar em paz
caso o usuário faça um Ctrl-C.

Linguagem da fsh
A linguagem compreendida pela fsh é bem simples. Cada sequência de caracteres diferentes de
espaço é considerada um termo. Termos podem ser
(i) operações internas da shell,
(ii) nomes de programas que devem ser executados (e seus argumentos),
(iii) operadores especiais

- Operações internas da shell são as sequências de caracteres que devem sempre ser executadas
pela própria shell e não resultam na criação de um novo processo. Na fsh as operações internas
são: cd, pwd, waitz e exit. Essas operações devem sempre terminar com um sinal de fim de linha
(return) e devem ser entradas logo em seguida ao prompt (isto é, devem sempre ser entradas como
linhas separadas de quaisquer outros comandos).

cd: Muda o diretório corrente da shell. Isso terá impacto sobre os arquivos visíveis sem um caminho completo (path).

pwd: Exibe o diretório corrente visto pelo processo.
waita: Faz com que a shell libere todos os processos filhos que estejam no estado “Zombie” antes de exibir um novo prompt. Cada processo que seja “encontrado” durante um waitz deve ser informado através de uma mensagem na linha de comando. Caso não haja mais processos no estado “Zombie”, uma mensagem a respeito deve ser exibida e fsh deve continuar sua execução.
exit: Este comando permite terminar propriamente a operação da shell. Ele faz com que todos os seus herdeiros vivos (herdeiros diretos e indiretos) morram também ... e a fsh só deve morrer após todos eles terem sido “liberados” do estado “Zombie”.

[...]

Ver mais em Especificação.pdf
