static void output_css(html_context* ctx)
{
	// Default universal settings
	print_str(ctx->f,
		// This adds support for light and dark modes based on browser settings
		":root {\n"
		"	color-scheme: light dark;\n"
		"}\n\n"

		"body {\n"
		// Set font and base size
		"	font-family: 'Georgia', serif;\n"
		"	font-size: 18px;\n"
		// Set theme colours
		"	color: light-dark(#000000, #E0E0E0);\n"
		"	background-color: light-dark(#FFFFFF, #1E1E1E);\n"
		// Centre text at a comfortable column width for reading
		"	max-width: 55ch;\n"
		"	margin-left: auto;\n"
		"	margin-right: auto;\n"
		// Add padding so we don't go up against the edges of small displays
		"	padding-left: 1em;\n"
		"	padding-right: 1em;\n"
		"	padding-bottom: 1em;\n"
		"}\n\n"

		// Chapter headings are centred
		"h1 {\n"
		"	text-align: center;\n"
		"}\n\n"

		// Title heading
		"h1.title {\n"
		"	font-size: 48px;\n"
		"	padding-top: 128px;\n"
		"	padding-bottom: 128px;\n"
		"}\n\n"

		// Superscript
		"sup {\n"
		"	line-height: 0;\n"		// Prevent references from increasing line height
		"	font-size: 0.75em;\n"
		"}\n\n"

		// Remove underlines from hyperlinks
		"a {\n"
		"	text-decoration: none;\n"
		"}\n\n"

		// Add underline when hovering over link
		"a:hover {\n"
		"	text-decoration: underline;\n"
		"}\n\n"

		// Links should not stand out when printing
		"@media print {\n"
		"	a {\n"
		"		color: black;\n"
		"	}\n"
		"}\n\n"

		// Avoid new lines after a heading when printing
		"h1, h2, h3 {\n"
		"	page-break-after: avoid;\n"
		"}\n\n"

		// Paragraphs which don't follow a heading are not indented
		"p {\n"
		"	margin-top: 0;\n"
		"	text-indent: 1.5em;\n"
		"	text-align: justify;\n"
		"	hyphens: auto;\n"
		"	margin-bottom: 0;\n"
		"}\n\n"

		// Right-aligned paragraphs
		"p.right-aligned {\n"
		"	margin-top: 1em;\n"
		"	text-align: right;\n"
		"}\n\n"

		// Paragraph with previous gap
		"p.paragraph-break {\n"
		"	margin-top: 1em;\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Authors
		"p.authors {\n"
		"	text-align: center;\n"
		"	padding-top: 0;\n"
		"	padding-bottom: 128px;\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// First footnote paragraph
		"p.footnote {\n"
		"	margin-top: 1.5em;\n"
		"	text-indent: 0;\n"
		"	font-size: 0.75em;\n"
		"}\n\n"

		// Subsequent footnote paragraph
		"p.footnote_paragraph {\n"
		"	text-indent: 1.5em;\n"
		"	font-size: 0.75em;\n"
		"}\n\n"

		// Footnotes following another footnote
		"p.footnote_paragraph + p.footnote {\n"
		"	margin-top: 1em;\n"
		"}\n\n"

		"p.footnote + p.footnote {\n"
		"	margin-top: 1em;\n"
		"}\n\n"

		// Paragraphs after headings are not indented
		"h1 + p,\n"
		"h2 + p,\n"
		"h3 + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Blockquote indentation
		"blockquote {\n"
		"	margin-left: 1.5em;\n"
		"}\n\n"

		// First paragraphs within a blockquote are not indented
		"blockquote p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Paragraphs following first blockquote paragraph are indented
		"blockquote p + p {\n"
		"	text-indent: 1.5em;\n"
		"}\n\n"

		// Paragraphs following blockquotes are not indented
		"blockquote + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Paragraphs following dinkuses are not indented
		"hr + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Lists
		"ol, ul {\n"
		"	text-align: justify;\n"
		"	hyphens: auto;\n"
		"	margin-left: 1.5em;\n"
		"	padding-left: 0;\n"
		"}\n\n"

		// Paragraphs following lists are not be indented
		"ol + p,\n"
		"ul + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Chapter list should be left-aligned
		"ul.chapters {\n"
		"	text-align: left;\n"
		"}\n\n"

		// Turn HR tags into dinkuses ("* * *")
		"hr {\n"
		"	border: none;\n"
		"	word-spacing: 1em;\n"
		"	margin-top: 1.5em;\n"
		"	margin-bottom: 1.5em;\n"
		"}\n\n"

		"hr::before {\n"
		"	color: light-dark(#000000, #E0E0E0);\n"
		"	content: '* * *';\n"
		"	display: block;\n"
		"	text-align:center;\n"
		"}\n"
	);
}