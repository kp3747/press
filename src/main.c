//static char skip_line(parse_context* ctx)
//{
//	char c;
//	do
//	{
//		c = get_char(ctx);
//	} while (c != '\n');
//
//	return get_char(ctx);
//}
//
//static char skip_comment(parse_context* ctx)
//{
//	char c = get_char(ctx);
//	if (c == '/')
//	{
//		// Skip past the end of the line for C++ comments
//		for (;;)
//		{
//			c = get_char(ctx);
//			if (c == '\n')
//				return get_char(ctx);
//		}
//	}
//	else if (c == '*')
//	{
//		for (;;)
//		{
//			c = get_char(ctx);
//			if (c == '\n' && ctx->read_ptr[0] == '*' && ctx->read_ptr[1] == '/')
//			{
//				c = consume_chars(ctx, 3);
//				if (c == '\n')
//				{
//					return get_char(ctx);
//				}
//				else
//				{
//					handle_parse_error(ctx, "Unexpected character after end of C-style (/*...*/) comment.");
//				}
//			}
//			else if (c == 0)
//			{
//				handle_parse_error(ctx, "Unexpected end of file inside C-style (/*...*/) comment.");
//			}
//		}
//	}
//	else
//	{
//		handle_parse_error(ctx, "Expected '/' or '*'.");
//	}
//}
//
//static char parse_paragraph(parse_context* ctx, char c)
//{
//	put_control_code(ctx, control_code_paragraph);
//
//	for (;;)
//	{
//		if (check_emphasis(ctx, c))
//		{
//		}
//		else if (check_dash(ctx, c))
//		{
//		}
//		else if (check_space(ctx, c))
//		{
//		}
//		else if (check_newline(ctx, c))
//		{
//			c = get_char(ctx);
//			if (c == '\n')
//				return get_char(ctx);
//			else
//				put_control_code(ctx, control_code_break);
//		}
//		else
//		{
//			put_char(ctx, c);
//		}
//
//		c = get_char(ctx);
//	}
//}
//
//static char parse_blockquote(parse_context* ctx, char c)
//{
//	put_control_code(ctx, control_code_blockquote_begin);
//
//	c = get_char(ctx);
//	for (;;)
//	{
//		if (check_emphasis(ctx, c))
//		{
//		}
//		else if (check_dash(ctx, c))
//		{
//		}
//		else if (check_space(ctx, c))
//		{
//		}
//		else if (check_newline(ctx, c))
//		{
//			c = get_char(ctx);
//			if (c == '\t')
//			{
//				c = get_char(ctx);
//				if (check_newline(ctx, c))
//				{
//				}
//				else
//				{
//					put_control_code(ctx, control_code_break);
//				}
//			}
//			else
//			{
//				return get_char(ctx);
//			}
//		}
//		else
//		{
//			put_char(ctx, c);
//		}
//
//		c = get_char(ctx);
//	}
//}
//
//static void parse(parse_context* ctx)
//{
//	ctx->line = 1;
//	ctx->column = 1;
//	ctx->next_line = 1;
//	ctx->next_column = 1;
//
//	char c = get_char(ctx);
//	for (;;)
//	{
//		switch (c)
//		{
//		case '#':
//			c = parse_heading(ctx, c);
//			break;
//		case ' ':
//			handle_parse_error(ctx, "Lines cannot begin with a space.");
//		case '\n':
//			handle_parse_error(ctx, "Unnecessary blank line.");
//		case '/':
//			c = skip_comment(ctx);
//			break;
//		case '\t':
//			c = parse_blockquote(ctx, c);
//			break;
//		case '[':
//		case '*':
//		case '1':
//		case '2':
//		case '3':
//		case '4':
//		case '5':
//		case '6':
//		case '7':
//		case '8':
//		case '9':
//		case 'I':
//		case 'V':
//		case 'X':
//			c = skip_line(ctx);
//			break;
//		case 0:
//			return;
//		default:
//			c = parse_paragraph(ctx, c);
//		}
//	}
//}

int main(void)
{
	//char* text = load_file("C:\\dev\\press\\doc\\Combat Liberalism - Mao Zedong.txt");

	return EXIT_SUCCESS;
}