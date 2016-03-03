# fsh
fsh é uma shell, que ao contrário das shells UNIX convencionais, ao processar um comando que seja um programa para ser executado ela deverá criar o processo para executar este programa em background. Outro diferencial da fsh é que ela permite que em uma mesma linha de comandos o usuário solicite a execução de vários programas em paralelo mas seguindo uma hierarquia.

## Como usar
Separe os nomes dos executáveis por "@"

```bash
$ fsh> firefox @ gedit
```
