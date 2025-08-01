* Todo-List:
 - *Terminar* class instances 'new Instance()'.
 - Adicionar suporte a referencias de instancia da class (mais em notes 0)
 - Adicionar suporte a super e constructors no codegen
 - Adicionar o sistema de IDs em classes (pra polimorfismo seguro em run-time).
 - Adicionar cast implicito em argumentos em function calls.
 - Adicionar suporte a statics no member access

* Notes:
 - Note 0:
   * Exemplo:
	class Test {
		private var: int = 0

		public fn get_var(): int {
			return var // usar a referencia da instancia + offset ([instance+offset])
		}
	}