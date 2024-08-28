#include "auth.h"

/* Funcția din server care transmite (sau nu) jetoane de autorizare */
char **req_authorization_1_svc(rpc_client *client, struct svc_req *cl)
{
	static char *res;
	res = (char *)malloc(MAX_LEN_LINE * sizeof(char));

	list_node *iter = rpc_sv->users_id->head;
	printf("BEGIN %s AUTHZ\n", client->user_id);

	/* Se parcurge lista pentru a verifica dacă
	   ID-ul se află în baza de date */
	while (iter)
	{
		/* Dacă se află, atunci se trimite clientului jetonul */
		if (!strcmp(iter->data, client->user_id))
		{
			strcpy(res, generate_access_token(client->user_id));
			printf("  RequestToken = %s\n", res);
			return &res;
		}
		iter = iter->next;
	}
	strcpy(res, "USER_NOT_FOUND");
	return &res;
}

/* Funcția din sever care întoarce (sau nu) jetonul de acces
   și de refresh (după caz) */
char **req_acc_token_2_svc(rpc_client *var, struct svc_req *cl)
{
	static char *res;
	res = (char *)malloc(MAX_LEN_LINE * sizeof(char));
	char acc_token[TOKEN_LEN], refresh_token[TOKEN_LEN];

	/* Se tratează cazul în care se folosește jetonul de refresh
	   pentru a genera unul de acces (și nu se mai cere aprobarea
	   utilizatorului) */
	if (var->refresh_token.is_signed == 2 && !var->acc_token.avl_op)
	{
		/* Se folosește funcția de generare din schelet în
		   ambele cazuri */
		printf("BEGIN %s AUTHZ REFRESH\n", var->user_id);
		strcpy(acc_token, generate_access_token(var->refresh_token.token_name));
		printf("  AccessToken = %s\n", acc_token);
		sprintf(res, "%s,%d", acc_token, avl_op);
		add_list_node(rpc_sv->acc_tokens, acc_token);

		/* Se construiește noul jeton de refresh folosind jetonul de acces */
		strcpy(refresh_token, generate_access_token(acc_token));
		printf("  RefreshToken = %s\n", refresh_token);
		sprintf(res, "%s,%s,%d", acc_token, refresh_token, avl_op);
	}
	else
	{
		/* Se verifică dacă utilizatorul a semnat jetonul de autorizare */
		if (var->auth_token.is_signed)
		{
			/* Se folosește funcția de generare din schelet în
			   ambele cazuri */
			strcpy(acc_token, generate_access_token(var->auth_token.token_name));
			printf("  AccessToken = %s\n", acc_token);
			sprintf(res, "%s,%d", acc_token, avl_op);
			add_list_node(rpc_sv->acc_tokens, acc_token);

			/* Se verifică dacă s-a optat pentru refacerea automată */
			if (atoi(*var->options.options_val))
			{
				strcpy(refresh_token, generate_access_token(acc_token));
				printf("  RefreshToken = %s\n", refresh_token);
				sprintf(res, "%s,%s,%d", acc_token, refresh_token, avl_op);
			}
		}
		else
			strcpy(res, "REQUEST_DENIED");
	}

	return &res;
}

int valid_permission(char *permission, rpc_client *var)
{
	if ((strchr(permission, *var->operation_type.operation_type_val[0]) ||
		 (strchr(permission, 'X') && *var->operation_type.operation_type_val[0] == 'E')) &&
		(*var->operation_type.operation_type_val &&
		 (!strcmp(*var->operation_type.operation_type_val, "READ") ||
		  !strcmp(*var->operation_type.operation_type_val, "MODIFY") ||
		  !strcmp(*var->operation_type.operation_type_val, "EXECUTE") ||
		  !strcmp(*var->operation_type.operation_type_val, "DELETE") ||
		  !strcmp(*var->operation_type.operation_type_val,
				  "INSERT"))))
		return 1;
	return 0;
}

/* Funcția din server care verifică operația respectivă */
char **validate_delegate_action_3_svc(rpc_client *var, struct svc_req *cl)
{
	static char *res;
	res = (char *)malloc(MAX_LEN_LINE * sizeof(char));
	char parser[MAX_LEN_LINE];

	/* Cazul în care jetonul a expirat */
	if (!var->acc_token.avl_op && var->auth_token.is_signed)
	{
		strcpy(res, "TOKEN_EXPIRED");
		printf("DENY (%s,%s,,0)\n", *var->operation_type.operation_type_val,
			   *var->options.options_val);
		return &res;
	}

	/* Se parcurge lista de jetoane de acces pentru a vedea dacă
	   un client folosește un jeton valid */
	iter_acc_tokens = rpc_sv->acc_tokens->head;
	if (iter_acc_tokens && iter_acc_tokens->data)
	{
		while (iter_acc_tokens)
		{
			/* Cazul în care există jetonul în baza de date */
			if (iter_acc_tokens->data &&
				!strcmp(iter_acc_tokens->data, var->acc_token.token_name))
			{

				/* Se verifică resursa de interes în baza de date */
				list_node *iter_resources = rpc_sv->resources->head;
				int exists = 0;
				while (iter_resources)
				{
					/* Dacă se găsește resursa, atunci se oprește căutarea */
					if (!strcmp(iter_resources->data, *var->options.options_val))
					{
						exists = 1;
						break;
					}
					iter_resources = iter_resources->next;
				}
				if (exists)
				{
					/* Se verifică permisiunea pe resursa găsită */
					strcpy(parser, var->auth_token.permissions);
					if (strstr(parser, *var->options.options_val))
					{
						/* Un oneliner care extrage permisiunile acelei resurse */
						char *permission = strtok(strstr(parser, *var->options.options_val) +
													  strlen(*var->options.options_val) + 1,
												  ",");

						/* Se verifică toate permisiunile pentru acea resursă */
						if (valid_permission(permission, var))
						{
							/* Se decrementează numărul de operații valabile */
							if (var->acc_token.avl_op > 0)
							{
								strcpy(res, "PERMISSION_GRANTED");
								int aux = var->acc_token.avl_op - 1;
								printf("PERMIT (%s,%s,%s,%d)\n",
									   *var->operation_type.operation_type_val,
									   *var->options.options_val, var->acc_token.token_name,
									   aux);

								/* Se șterge jetonul de acces din baza de date atunci
								   când expiră */
								if (!aux)
									remove_list_node(rpc_sv->acc_tokens,
													 var->acc_token.token_name);
							}
							return &res;
						}
						else
						{
							strcpy(res, "OPERATION_NOT_PERMITTED");
							break;
						}
					}
					else
					{
						strcpy(res, "OPERATION_NOT_PERMITTED");
						break;
					}
				}
				else
				{
					strcpy(res, "RESOURCE_NOT_FOUND");
					break;
				}
			}
			iter_acc_tokens = iter_acc_tokens->next;
		}
	}
	else
	{
		/* Cazul în care se încearcă accesul fără un jeton de acces valid */
		strcpy(res, "PERMISSION_DENIED");
		printf("DENY (%s,%s,,0)\n", *var->operation_type.operation_type_val,
			   *var->options.options_val);
	}

	/* Se decrementează numărul de operații valabile și în situația
	   în care operația a eșuat (din diverse motive) */
	if (var->acc_token.avl_op > 0)
	{
		int aux = var->acc_token.avl_op - 1;
		printf("DENY (%s,%s,%s,%d)\n", *var->operation_type.operation_type_val,
			   *var->options.options_val, var->acc_token.token_name, aux);

		/* Se șterge jetonul de acces din baza de date atunci
		   când expiră */
		if (!aux)
			remove_list_node(rpc_sv->acc_tokens, var->acc_token.token_name);
	}
	return &res;
}

/* Funcția din server care aprobă (sau nu) cererea unui client */
char **approve_request_token_4_svc(rpc_client *var, struct svc_req *cl)
{
	static char *res;
	res = (char *)malloc(MAX_LEN_LINE * sizeof(char));

	strcpy(res, var->auth_token.token_name);

	/* Dacă se aprobă, atunci câmpul is_signed primește valoarea 1
	   Dacă nu, atunci se returnează jetonul neschimbat */
	if (strcmp(iter_users_perm->data, "*,-"))
		sprintf(res, "%s|%d", iter_users_perm->data, 1);

	iter_users_perm = iter_users_perm->next;
	return &res;
}
