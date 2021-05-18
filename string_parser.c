/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan
 *
 */

#include "string_parser.h"

int count_token (char* buf, const char* delim)
{
	/*
	*	#1.	Check for NULL string
	*	#2.	iterate through string counting tokens
	*		Cases to watchout for
	*			a.	string start with delimeter
	*			b. 	string end with delimeter
	*			c.	account NULL for the last token
	*	#3. return the number of token (note not number of delimeter)
	*/
	if (!*buf) {
		return 0;
	}
	
	int i = 0; 
	int count = 1;
	int prev = 0;
	while (buf[i] != '\0') {
		if (buf[i] == *delim) {
			if (i == 0) {
				i++;
				continue;
			}
			count++;
			prev = 1;
		} else {
			prev = 0;
		}
		i++;
	}
	
	if (prev) {
		count--;
		return count;
	} else {
		return count;
	}
}

command_line str_filler (char* buf, const char* delim)
{
	/*
	*	#1.	create command_line variable to be filled
	*	#2.	use function strtok_r to take out \n character then count
	*			number of tokens with count_token function, set num_token.
	*	#3. malloc memory for token array inside command_line variable
	*			base on the number of tokens.
	*	#4. malloc each index of the array with the length of tokens,
	*			fill array with tokens, and fill last spot with NULL.
	*	#5. return the variable.
	*/	
	command_line command;
	
	char* buf_cpy = strtok_r(buf, "\n", &buf);

	command.num_token = count_token(buf_cpy, delim);
		
	command.command_list = malloc((sizeof(char*) * command.num_token) + sizeof(NULL));

	int i = 0;
	char* token;
	while ((token = strtok_r(buf_cpy, delim, &buf_cpy))) {
		if (i == command.num_token) {
			break;
		} else if (*token) {
			*(command.command_list + i) = malloc(strlen(token) + 1);
			strcpy(*(command.command_list + i), token);
			i++;
		}
	}
	*(command.command_list + command.num_token) = NULL;
	
	return command;
}


void free_command_line(command_line* command)
{
	/*
	*	#1.	free the array base num_token
	*/
	for (int i = 0; i < command->num_token; i++) {
		free(*(command->command_list + i));
	}
	free(command->command_list);
}

