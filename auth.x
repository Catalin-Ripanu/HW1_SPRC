
/* Lungimea unui token ce include și terminatorul de șir */
const TOKEN_LEN = 16;

/* Lungimea maximă a unei linii dintr-un fișier text */
const MAX_LEN_LINE = 1000;

/* Un tip de date ajutător pentru crearea unui vector de șiruri */
typedef string string_l<>;

/* Tipul nod ce reține un șir de informație */
struct list_node
{
    struct list_node *next;
    struct list_node *prev;
    string data<>;
};

/* Tipul listă ce modelează lista înlănțuită de informații */
struct linked_list
{
    struct list_node *head;
    struct list_node *tail;
};

/* Tipul token ce modelează noțiunea de jeton (fie el de autorizare,
   de acces sau de refresh)
   Câmpul avl_op reprezintă numărul de operații ce se mai pot efectua
   Câmpul is_signed reprezintă întregul ce va indica dacă semnarea
   de către utilizatorul final a avut loc sau nu
   Câmpul permissions reprezintă șirul ce reține permisiunile separate
   prin virgulă
*/
struct rpc_client_token
{
    int avl_op;
	int is_signed;
	char permissions[MAX_LEN_LINE];
    char token_name[TOKEN_LEN];
};

/* Tipul ce modelează noțiunea de client în modelul Oauth
   Câmpul operation_type reține tipul operației cererii respective
   Câmpul options reține opțiunile asociate operației respective
   Câmpul operations stochează denumirea operației în cauză
   Ultimele 3 câmpuri reprezintă jetoanele specifice clientului
*/
struct rpc_client
{
    char user_id[MAX_LEN_LINE];
	string_l operation_type<>;
	string_l options<>;
    string_l operations<>;
	struct rpc_client_token auth_token;
    struct rpc_client_token acc_token;
	struct rpc_client_token refresh_token;
};


program AUTH_PROG {

	/* Funcția de Request Authorization din modelul Oauth care verifică
	   existența utilizatorului în baza de date
	*/
	version REQ_AUTH_VERSION {
		string REQ_AUTHORIZATION(rpc_client*) = 1;
	} = 1;

	/* Funcția de Request Access Token din modelul Oauth care verifică
	   dacă un jeton este valid (prin semnătură)
	*/
	version REQ_ACC_VERSION {
		string REQ_ACC_TOKEN(rpc_client*) = 1;
	} = 2;

	/* Funcția de Validate Delegated Action din modelul Oauth care verifică 
	   dacă un jeton este valid (prin semnătură) pentru a executa o acțiune
	*/
	version VAL_DELG_VERSION {
		string VALIDATE_DELEGATE_ACTION(rpc_client*) = 1;
	} = 3;

	/* Funcția de Validate Delegated Action din modelul Oauth care modifică
	   (sau nu) jetonul de autorizare
	*/
	version APPR_REQ_VERSION {
		string APPROVE_REQUEST_TOKEN(rpc_client*) = 1;
	} = 4;
} = 123456789;
