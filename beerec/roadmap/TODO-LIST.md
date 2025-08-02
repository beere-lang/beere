* Todo-List:
 - Fixar o uso de variaveis do style de class globals em qualquer escopo dentro de classes (mais em note 0)
 - Fixar o registrador usado na instanciação de classes sendo overwrite em outras instanciações feitas nesse meio tempo.
 - Testar totalmente classes pra checar bugs, etc.
 - Adicionar suporte a statics no member access
 - Refatorar o parser e analyzer
 - Adicionar o sistema de IDs em classes (pra polimorfismo seguro em run-time).
 - Adicionar cast implicito em argumentos em function calls.

* Notes:
Note 0:
```rs
class Test {
    fn style() {
        public field: int = 0 // ISSO É ACEITO, NÃO DEVERIA. CORRETO: 'let field: int = 0'
    }
}
```