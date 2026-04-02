#ifndef COMMANDS_H
#define COMMANDS_H

#include "ui/ui.h"
#include "util/sutil.h"

int eval(BString *bs, ui_t *ui);
int process_command_line(ui_t *ui, const char *command);

#endif // COMMANDS_H
