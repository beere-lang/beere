* Todo-List:
 - Testar o funcionamento de visibilidade (private/public) em classes e supers (não testado)
 - Implementar o acesso de variaveis da propria instancia sem uso de 'this' ptr - *Note 0*
 - Testar totalmente classes pra checar bugs, etc.
 - Adicionar suporte a statics no member access e nas classes
 - Refatorar o parser e analyzer
 - Adicionar o sistema de IDs em classes (pra polimorfismo seguro em run-time).
 - Adicionar cast implicito em argumentos em function calls.

* Notes:
Note 0:
```ts
class Test {
	public teste: int = 0
	
	public Test() {
		teste = 10 // usa o teste da propria classes (não precisa de this)
	}
}
```