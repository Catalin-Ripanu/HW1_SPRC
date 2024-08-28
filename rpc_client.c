#include "auth.h"

/* Funcția de inițializare a clienților */
void init_clients(rpc_client *clients, int *op_len, int clients_nr)
{
	/* Se alocă memorie pentru câmpul operațiilor / cererilor */
	for (int i = 0; i < clients_nr; i++)
	{
		op_len[i] = 0;
		clients[i].operations.operations_val = (string_l *)calloc(MAX_LEN_LINE, sizeof(string_l));
		if (!clients[i].operations.operations_val)
		{
			printf("calloc failed in init_clients function\n");
			exit(1);
		}
		strcpy(clients[i].refresh_token.token_name, "");
		strcpy(clients[i].acc_token.token_name, "");
		strcpy(clients[i].auth_token.permissions, "");
		clients[i].auth_token.is_signed = 0;
		clients[i].acc_token.is_signed = 0;
		clients[i].refresh_token.is_signed = 0;
		clients[i].acc_token.avl_op = 0;
		clients[i].refresh_token.avl_op = 0;
	}
}

/* Funcția care alocă memorie pentru a pregăti clienții */
void prepare_clients(string_l *operations_val, int operations_len, list_node *iter, linked_list *names,
					 rpc_client *clients, int *op_len, int clients_nr, char *parser, int index)
{
	/* Mai multe detalii despre această logică
	   se vor găsi în README */
	for (int i = 0; i < operations_len; i++)
	{
		/* Se atribuie fiecărui client operațiile / cererile
		   de interes prin popularea câmpurilor structurilor */
		strcpy(parser, operations_val[i]);
		iter = names->head;
		index = 0;

		/* Pentru fiecare cerere extrasă se parcurge lista de ID-uri
		   pentru a se găsi clientul potrivit */
		while (iter)
		{
			if (!strcmp(iter->data, strtok(parser, ",")))
			{
				/* Clientul cu indexul 'index' primește cererea 'operations_val[i]' din fișier */
				clients[index].operations.operations_val[op_len[index]] = strdup(operations_val[i]);

				/* Programare defensivă */
				if (!clients[index].operations.operations_val[op_len[index]])
				{
					printf("strdup failed in prepare_clients function\n");
					exit(1);
				}
				op_len[index] += 1;
				break;
			}
			index++;
			iter = iter->next;
		}
	}

	/* Partea care populează cu informații câmpurile structurilor */
	for (int i = 0; i < clients_nr; i++)
	{
		clients[i].options.options_len = op_len[i];
		clients[i].options.options_val = (string_l *)calloc(op_len[i], sizeof(string_l));

		/* Programare defensivă */
		if (!clients[i].options.options_val)
		{
			printf("calloc failed in prepare_clients function for options_val\n");
			exit(1);
		}

		clients[i].operation_type.operation_type_len = op_len[i];
		clients[i].operation_type.operation_type_val = (string_l *)calloc(op_len[i], sizeof(string_l));

		/* Programare defensivă */
		if (!clients[i].operation_type.operation_type_val)
		{
			printf("calloc failed in prepare_clients function for operation_type_val\n");
			exit(1);
		}

		clients[i].operations.operations_len = op_len[i];
		clients[i].operations.operations_val = (string_l *)realloc(clients[i].operations.operations_val,
																   op_len[i] * sizeof(string_l));

		/* Programare defensivă */
		if (!clients[i].operations.operations_val)
		{
			printf("realloc failed in prepare_clients function for operations_val\n");
			exit(1);
		}

		/* Se parsează șiruri astfel încât fiecare subșir să fie
		   poziționat în câmpul potrivit */
		for (int j = 0; j < clients[i].operations.operations_len; j++)
		{
			strcpy(parser, clients[i].operations.operations_val[j]);
			strcpy(clients[i].user_id, strtok(parser, ","));
			clients[i].operation_type.operation_type_val[j] = strdup(strtok(NULL, ","));

			/* Programare defensivă */
			if (!clients[i].operation_type.operation_type_val[j])
			{
				printf("strdup failed in prepare_clients function for operations_type_val\n");
				exit(1);
			}

			clients[i].options.options_val[j] = strdup(strtok(NULL, ","));

			/* Programare defensivă */
			if (!clients[i].options.options_val[j])
			{
				printf("strdup failed in prepare_clients function for operationsval\n");
				exit(1);
			}
		}
		op_len[i] = 0;
	}
}

/* Funcția de refacere a jetonului fără aprobarea utilizatorului */
void initiate_acc_token(char **res, rpc_client *clients, rpc_client client, CLIENT **handles,
						int index, int *op_len)
{
	strcpy(*client.options.options_val,
		   clients[index].options.options_val[op_len[index]]);

	/* Se folosește o convenție astfel încât server-ul să știe
	   că se va folosi un jeton de refresh pentru a forma
	   un jeton de acces */
	client.refresh_token.is_signed = 2;
	res = req_acc_token_2(&client, handles[1]);

	/* Se populează cu informații noi ambele tipuri jetoane*/
	strcpy(clients[index].acc_token.token_name, strtok(*res, ","));
	strcpy(clients[index].refresh_token.token_name, strtok(NULL, ","));

	clients[index].acc_token.avl_op = atoi(strtok(NULL, ","));
	clients[index].refresh_token.avl_op = clients[index].acc_token.avl_op;

	free(*res);
}

/* Funcția de validare a unei cereri din partea clientului */
void validate_action(char **res, rpc_client *clients, rpc_client client, CLIENT **handles,
					 int index, int *op_len)
{
	/* Se pregătește clientul cu informațiile necesare în acest caz */
	client = clients[index];
	strcpy(*client.operation_type.operation_type_val,
		   clients[index].operation_type.operation_type_val[op_len[index]]);
	strcpy(*client.options.options_val,
		   clients[index].options.options_val[op_len[index]]);
	strcpy(client.user_id, "");

	/* Se transmite cererea către server */
	res = validate_delegate_action_3(&client, handles[2]);

	/* Dacă nu a expirat jetonul, se decrementează numărul de operații valabile */
	if (strcmp(*res, "TOKEN_EXPIRED"))
		clients[index].acc_token.avl_op--;

	printf("%s\n", *res);

	free(*res);
}

/* Funcția de procesare a unei cereri din partea unui client */
void proccess_request(char **res, rpc_client *clients, CLIENT **handles, rpc_client client,
					  int index, int *op_len)
{
	/* Se formează jetonul de autorizare */
	res = req_authorization_1(&clients[index], handles[0]);

	/* Se transferă informația în câmpul token_name */
	memcpy(clients[index].auth_token.token_name, *res, TOKEN_LEN);
	clients[index].auth_token.token_name[strlen(clients[index].auth_token.token_name)] = '\0';

	/* Se verifică dacă ID-ul a existat sau nu în baza de date a server-ului */
	if (strcmp(*res, "USER_NOT_FOUND"))
	{
		free(*res);

		/* Se verifică dacă utilizatorul permite sau nu accesul */
		res = approve_request_token_4(&clients[index], handles[3]);

		/* Cazul în care a permis accesul */
		if (strchr(*res, '|'))
		{
			/* Se atribuie permisiunile respective jetonului */
			strcpy(clients[index].auth_token.permissions, strtok(*res, "|"));

			/* Se efectuează semnarea simbolică */
			clients[index].auth_token.is_signed = atoi(strtok(NULL, "|"));
		}
		else
			clients[index].auth_token.is_signed = 0;

		free(*res);

		client = clients[index];
		strcpy(*client.options.options_val,
			   clients[index].options.options_val[op_len[index]]);
		res = req_acc_token_2(&client, handles[1]);

		/* Cazul în care este permis accesul (se verifică în server
		   dacă jetonul a fost semnat) */
		if (strcmp(*res, "REQUEST_DENIED"))
		{
			strcpy(clients[index].acc_token.token_name, strtok(*res, ","));

			/* Se verifică dacă s-a optat pentru refacerea automată */
			if (atoi(clients[index].options.options_val[op_len[index]]))
			{
				strcpy(clients[index].refresh_token.token_name, strtok(NULL, ","));
				printf("%s -> %s,%s\n", clients[index].auth_token.token_name,
					   clients[index].acc_token.token_name, clients[index].refresh_token.token_name);

				/* Se inițializează jetonul de refresh */
				clients[index].refresh_token.is_signed = 1;
			}
			else
				printf("%s -> %s\n", clients[index].auth_token.token_name, clients[index].acc_token.token_name);

			/* Se atribuie numărul de operații ce se pot face cu acest jeton */
			clients[index].acc_token.avl_op = atoi(strtok(NULL, ","));
			if (strcmp(clients[index].refresh_token.token_name, ""))
				clients[index].refresh_token.avl_op = clients[index].acc_token.avl_op;
		}
		else
			printf("%s\n", *res);
	}
	else
		printf("%s\n", *res);
	free(*res);
}

int main(int argc, char *argv[])
{
	/* Variabilele ce vor ajuta la întreaga logică a clientului Oauth*/
	CLIENT *handles[4];
	char parser[MAX_LEN_LINE], **res = NULL;
	linked_list *names;
	string_l *operations_val;
	u_int operations_len;
	rpc_client client;
	int clients_nr = 0, index = 0;

	/* O condiție necesară pentru a executa clientul ce implementează modelul Oauth */
	if (argc != 3)
	{
		printf("[CLIENT] Pass 3 correct arguments for this Oauth client...\n");
		exit(1);
	}

	FILE *client_file = fopen(argv[2], "r");

	/* Se creează câte o rutină de execuție pentru fiecare funcție expusă de server */
	for (int i = 0; i < 4; i++)
		handles[i] = clnt_create(argv[1], AUTH_PROG, i + 1, "tcp");

	/* Se citesc ID-urile unice ale utilizatorilor
	   Acest names este necesar pentru a separa utilizatorii
	   în funcție de ID-urile lor */
	names = read_file_char_array(client_file, &operations_val, &operations_len);
	list_node *iter = names->head;

	/* Numărul utilizatorilor din fișier */
	while (iter)
	{
		clients_nr++;
		iter = iter->next;
	}

	/* Vectorii care vor ține informațiile necesare în cazul fiecărui client */
	rpc_client clients[clients_nr];
	int op_len[clients_nr];

	/* Se inițializează clienții */
	init_clients(clients, op_len, clients_nr);

	/* Se pregătesc clienții prin alocări de memorie și copieri de informații de început */
	prepare_clients(operations_val, operations_len,
					iter, names, clients, op_len, clients_nr, parser, index);

	/* Se iterează prin lista de cereri pentru procesarea fiecăreia
	   Mai multe detalii despre această logică se vor găsi în README
	   */
	for (int i = 0; i < operations_len; i++)
	{
		/* Se extrage operația / cererea curentă */
		strcpy(parser, operations_val[i]);

		/* Se caută clientul respectiv care a inițializat cererea */
		iter = names->head;
		index = 0;
		while (iter)
		{
			/* S-a găsit clientul pe baza ID-ului din lista names */
			if (!strcmp(iter->data, strtok(parser, ",")))
			{
				/* Se reține clientul de interes */
				client = clients[index];

				/* Se verifică dacă acest client de interes dorește să facă un REQUEST
				   pentru generarea / refacerea unui jeton de acces */
				if (!strcmp(clients[index].operation_type.operation_type_val[op_len[index]], "REQUEST"))
					proccess_request(res, clients, handles, client, index, op_len);
				else
				{
					/* Se verifică dacă a expirat jetonul de acces și dacă s-a optat pentru
					   refacerea automată folosind câmpul is_signed
					   Se folosește direct funcția de req_acc_token_2 */
					if (!clients[index].acc_token.avl_op && clients[index].refresh_token.is_signed)
						initiate_acc_token(res, clients, client, handles, index, op_len);

					/* Se transmite operația dorită către server (acesta o va valida)*/
					validate_action(res, clients, client, handles, index, op_len);
				}

				/* Se incrementează op_len[index] pentru a procesa operația următoare
				   atunci când se va ajunge la același client în fișierul de la intrare
				   Acest lucru este necesar deoarece clienții sunt dispersați */
				op_len[index]++;
				break;
			}
			index++;

			/* Se trece la următorul ID */
			iter = iter->next;
		}
	}

	/* Se dezalocă toată memoria alocată dinamic */
	for (int i = 0; i < operations_len; i++)
		free(operations_val[i]);
	free(operations_val);

	for (int i = 0; i < clients_nr; i++)
	{
		for (int j = 0; j < clients[i].operations.operations_len; j++)
		{
			free(clients[i].operations.operations_val[j]);
			free(clients[i].operation_type.operation_type_val[j]);
			free(clients[i].options.options_val[j]);
		}
		free(clients[i].operations.operations_val);
		free(clients[i].operation_type.operation_type_val);
		free(clients[i].options.options_val);
	}
	destroy_list(names);

	return 0;
}
