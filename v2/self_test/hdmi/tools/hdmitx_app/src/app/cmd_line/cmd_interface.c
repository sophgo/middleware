// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/cmd_line/cmd_interface.h"
#include "includes.h"
#include "hdmitx_ipk_api/video_bridge/video_bridge.h"
#include "hdmitx_ipk_api/audio_bridge/audio_bridge.h"
#include "platform.h"

/**
 * Prototypes
 */
int execute_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd);

/**
 * @short Check the application flags with the command option
 * @param[in,out] app Main Application structure
 * @param[in,out] cmd Command list
 * return void
 */
int _check_flags(struct hdmi_tx_app *app, enum command_flags flags)
{
	if(!flags) //No flags defined
		return TRUE;
	if(flags & app->flags){ //flag defined in app configuration
		return TRUE;
	}

	return FALSE;
}

/**
 * Implementation
 */
int get_cmd_line(struct hdmi_tx_app *app, char *cmd)
{
	// get characters from the user and add them to a temporary buffer
	// copy the content of that buffer to cmd
	bool exit = false;
	char cmd_data[CMD_SIZE];
	int cmd_idx = 0;

	memset(cmd_data, 0, sizeof(cmd_data));
	
	if(app->current_menu != NULL)
		printf("[hdmitx%s]# ", app->current_menu[0].command);
	else
		printf("[hdmitx]# ");

	while(!exit){
		int command = getchar();
		printf("command: %d\n",command);
		putchar(command);
		switch(command){
		case '\n':
			memcpy(cmd, cmd_data, CMD_SIZE);
			exit = true;
			break;
		case 0x8: // backspace
			cmd_data[cmd_idx--] = 0;
			break;
		default:
			if((cmd_idx) >= CMD_SIZE - 1)
				continue;
			cmd_data[cmd_idx++] = (char)command;
			break;
		}
	}
	return 1;
}

/**
 * @short Parse command line string
 * @param[in,out] app Main Application structure
 * @param[in] cmd Command
 * return Pointer to cmd_list_t. Returns the full list of commands in a linked list.
 */
cmd_list_t * parse_cmd_line(struct hdmi_tx_app *app, char *cmd)
{
	char *cmd_temp = NULL;
	char *cmd_token = NULL;
	char *params_token = NULL;
	cmd_list_t *first_entry = NULL;
	cmd_list_t *last_entry = NULL;

	cmd_temp = strsep(&cmd, ";");

	while (cmd_temp != NULL){

		// Get command
		cmd_token = strsep(&cmd_temp, " ");
		if(strlen(cmd_token) == 0) //remove write spaces
			continue;

		cmd_list_t *entry = malloc(sizeof(cmd_list_t));

		if(last_entry == NULL){
			first_entry = entry;  
		}else {
			last_entry->next = entry;
		}

		entry->first = first_entry;

		// Get parameters
		params_token = strsep(&cmd_temp, ";");

		// Update the first command and parameters
		entry->command = cmd_token;
		entry->params = params_token;
		entry->next = NULL;

		last_entry = entry;

		cmd_temp = strsep(&cmd, ";");
	}
	return first_entry;
}

/**
 * @short Free command list
 * @param[in] cmd_list Command list
 * return void
 */
void free_cmd_list(cmd_list_t *cmd_list)
{
	cmd_list_t *aux_list = cmd_list;
	cmd_list_t *next_list = cmd_list;

	while (aux_list != NULL){
		next_list=aux_list->next;
		free(aux_list);
		aux_list = NULL;
		aux_list = next_list;
	}
}

int execute_cmd_list(struct hdmi_tx_app *app, cmd_list_t *cmd_list)
{
	cmd_list_t *current_cmd = cmd_list;

	if((app == NULL) || (app->current_menu == NULL)|| (cmd_list == NULL))
		return -ENXIO;

	while(current_cmd != NULL){
				execute_cmd(app, current_cmd);
		current_cmd = current_cmd->next;
	}
	return 1;
}

int execute_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	menu_t * menu_table = app->current_menu;
	int i = 0;


	if(menu_table == NULL)
		return -EINVAL;

	if(strlen(cmd->command) == 0)
		return 0;

	for(i = 0; menu_table[i].type != END_MARK; i++){
		if(command_match(cmd->command, menu_table[i].command)){
			switch ( menu_table[i].type){
				case MENU:
					if(menu_table[i].sub_menu != NULL && (_check_flags(app,menu_table[i].flags) == TRUE))
						app->current_menu = menu_table[i].sub_menu;
					else
						printf("Invalid Menu option\n");
					break;
				case ACTION:
					if(menu_table[i].command_function != NULL)
						menu_table[i].command_function(app, cmd);
					else
						printf("Invalid command\n");
					break;
				case EXIT_MENU:
					if(menu_table[i].sub_menu == NULL)
						exit(0);
					else
						app->current_menu = menu_table[i].sub_menu;
					break;
				case END_MARK:
				case BANNER_MARK:
				default:
					printf("Invalid command\n");
			}
			break;
		}
	}
	return 0;
}




/**
 * @short Print the list of commands
 * @param[in,out] app Main Application structure
 * @param[in,out] cmd Command list
 * return void
 */
void help_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{
	int i = 0;
	menu_t *list = app->current_menu;

	for(i = 0; list[i].type != END_MARK; i++){
		if (_check_flags(app,list[i].flags) == FALSE) //should be displayed
			continue;

		if(list[i].type == BANNER_MARK)
			printf(" -%s-\n", list[i].description);
		else
			printf("\t%-20.20s - %s\n", list[i].command, list[i].description);
	}
}

/**
 * @short Enable the expert mode
 * @param[in,out] app Main Application structure
 * @param[in,out] cmd Command list
 * return void
 */
void expert_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd)
{

	if((app == NULL) || (cmd  == NULL)){
		CMD_LOGGER(SNPS_ERROR, "Invalid command");
		return;
	}

	if (cmd->params == NULL) {
		CMD_LOGGER(SNPS_ERROR, "Invalid command");
		return;
	}

	if(command_match(cmd->params, "Enable")){
		app->flags |= CMD_EXPERT;
	}
	else if(command_match(cmd->params, "Disable")){
		app->flags &= ~(CMD_EXPERT);
	}
	else{
		CMD_LOGGER(SNPS_ERROR, "Invalid command");
		return;
	}
}

/**
 * @short Get capitals from string
 * @param[in] string String to parse
 * @param[out] caps Pointer for the capital letters buffer
 * return TRUE
 */
int get_string_caps(const char * string, char * caps)
{
	int i , j = 0;
	int len = strlen(string);
	for(i = 0; i < len; i++){
		if(string[i] >= 'A' && string[i] <= 'Z')
			caps[j++] = string[i];
		else if(string[i] >= '0' && string[i] <= '9')
			caps[j++] = string[i];
	}

	caps[j] = '\0';
	return TRUE;
}

/**
 * @short Check if the command matches
 * @param[in] command Command
 * @param[in] operation Operation
 * return TRUE if match or FALSE if not match
 */
int command_match(const char * command,const char * operation)
{
	char cmdCaps[15];

	if(command == NULL)
		return FALSE;

	if((strcasecmp(command, operation) == 0))
		return TRUE;
	get_string_caps(operation, cmdCaps);
	if(strcasecmp(command,cmdCaps) == 0)
		return TRUE;
	return FALSE;
}
