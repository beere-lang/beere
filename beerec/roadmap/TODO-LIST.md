* Todo-List:
 - Fixar referencias da stack, etc.
 - Fazer functions calls voltarem o tanto que foi adicionado de argumentos (push rax, push rbx --> add rsp, 16 * sobe o espaço que alocou pros argumentos)
 - Testar totalmente classes pra checar bugs, etc.
 - Adicionar suporte a referencias de instancia da class (mais em notes 0)
 - Adicionar suporte a statics no member access
 - Refatorar o parser e analyzer
 - Adicionar suporte a super e constructors no codegen
 - Adicionar o sistema de IDs em classes (pra polimorfismo seguro em run-time).
 - Adicionar cast implicito em argumentos em function calls.

* Notes:
 - Note 0:
   * Exemplo:
	class Test {
		private var: int = 0

		public fn get_var(): int {
			return var // usar a referencia da instancia + offset ([instance+offset])
		}
	}