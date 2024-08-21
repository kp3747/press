# Overview

The purpose of the press tool is to allow the generation of professional documents with no technical knowledge besides the formatting specification presented in this document. Multiple output formats can be generated from the same input text file. The currently supported output formats are:

1. [HTML](https://en.wikipedia.org/wiki/HTML)/[CSS](https://en.wikipedia.org/wiki/CSS) webpage
2. [ePub](https://en.wikipedia.org/wiki/EPUB) eBook

Printable formats such as the [Open Document Format](https://en.wikipedia.org/wiki/OpenDocument) (.odt) will be supported in the future. This format is supported by most word processors including LibreOffice, OpenOffice, and Microsoft Word.

# Rationale

The initial impetus for developing this application was the time consuming formatting of documents in traditional word processors, which also often ended up creating inconsistent results. The problem with [WYSIWYG](https://en.wikipedia.org/wiki/WYSIWYG) editors is that they mix the paradigms of semantic and visual design. Semantics refer to the context of an element such as a heading, paragraph, or blockquote. Using a visual editor means making something *look* like a blockquote, but the application has no way to know that. Word processors allow you to mark text semantically using styles, but they also allow you to easily circumvent them.

When creating a document in a single format this wouldn't be more than an inconvenience. We have recently, however, been generating multiple documents from the same text. We might want a webpage for the website, a printable PDF for sharing, a brochure for printing, and an eBook for reading on the go. We may also want to support printing in A4 or letter sizes depending on the location of readers. Making changes to the core text meant going back and updating all the other versions of the documents manually. Even worse than that, any style changes we make in the future would need to be applied retroactively to all the old documents.

Instead, we have created a custom tool that parses all the text semantically, and adjusts the visual aspects to suit the generated document formats. Updating the visual style for all documents just requires running the tool again.

This idea is similar to existing formats such as [LaTeX](https://en.wikipedia.org/wiki/LaTeX) and document generators using the [Markdown](https://en.wikipedia.org/wiki/Markdown) format. Why not use existing tools?

1. To ensure source text is readable in and of itself with as little metadata as possible.
2. To avoid dependencies. This tool is written in standard and cross-platform C with zero dependencies on other projects or libraries. It's just a single .exe file that does not require any installation procedures.
3. To keep the code small and simple. This allows us to easily add any features we need.

# Usage

The following example shows how to use the tool to generate a webpage.
`press --html "source-file.txt"`

The order of arguments does not matter, but the source file path is mandatory. All other parameters are optional. Passing just the source file path will validate the file without generating any documents.

Parameters:

* Source file path. If there are spaces in the path you should surround it in "double quote characters".
* --all - Generates all formats.
* --html - Generates an HTML webpage and CSS stylesheet.
* --epub - Generates an ePub eBook.

# Format

The format is very similar to [Markdown](https://www.markdownguide.org/cheat-sheet/). Differences have been introduced to simplify by removing unnecessary features, making the raw text more readable, or adding a specialised feature for our requirements.

Ultimately the input format is a raw text file, with a strict layout that allows for reading with consistent formatting, and for making the semantics of the text clear to the press tool.

The press tool does not combine multiple lines together. This means that each element is a single line. Paragraphs can become very long, so it is advised to turn on word-wrapping in your text editor. For example:

```
Pretend this is a really long paragraph.

There
will
be
a
line
break
between
each
of
these
lines.
```

## Headings

There are three levels of headings. In an article the top-level heading will be the title, while in a book it will be the chapter name. The other two headings should be used to subdivide the text.

Heading are defined by using the hash '#' character at the start of a line surrounded by blank lines. Subsequent heading levels add extra hashes.

```
# Article Title or Chapter Name

## Second Level

### Third level
```

## Paragraphs

A paragraph is a line of text surrounded by blank lines. New lines followed by text will be treated as a line break within the same paragraph. Two blank lines between paragraphs create a paragraph break. When paragraphs are formatted without blank lines between them, paragraph breaks will force one.

```
This is a paragraph.

This paragraph contains a
break.

These paragraphs


are separated by a paragraph break.
```

## Lists

There are two types of lists, ordered and unordered. Both must be placed at the beginning of a line surrounded by black lines.

Ordered lists can use arabic numbers, roman numerals, or letters. Each number or letter must be followed by a dot and exactly one space ". ".

Unordered lists are usually rendered using bullet points. They are defined using the asterisk character.

```
1. Apple
2. Orange
3. Pear

I. Apple
II. Orange
III. Pear

a. Apple
b. Orange
c. Pear

* Apple
* Orange
* Pear
```

## Quotes

Inline quotes are formatted using double quotation marks, and inner quotes are formatted using backticks (the key under the escape key on US and UK keyboards.

```
"This is a quote"

"This is a first-level quote, `and this is a second-level quote`."
```

## Block Quotes

A block quote is created by tabbing the paragraph or empty lines. They may optionally end with a citation created by a tab and em dash "---".

```
    This is the first paragraph of the quote.

    This is the second.

    ---This is the citation
```

## Text Decoration

Emphasised text is surrounded by a single asterisk "*" on each side. It is generally rendered as italicised text. Strong text is surrounded by two asterisks and is generally rendered as bold text.

```
*italicized text*
**bold text**
```

## Dashes

Various dashes are used in publishing that are no available on the keyboard. The press markup uses mulitple hypens "-" to specify them.

```
This is a hyphen "-".
This is an en dash "--".
This is an em dash "---".
```

## Comments

Comments allow text to be inserted in the source text file which is removed from generated documents. Example uses of comments are to temporarily remove text during editing, or to share comments between author and editor. There are two supported types of comments.

1. Pairs of "/*" and "*/" denote a multiline range comment. Range comments can be placed anywhere, but careless use can add extra spaces which can in turn affect the semantics of the text.
2. "//" can only be used at the start of a line. It does not introduce a new line so has no effect on the text semantics.

```
// Commentting on following paragraph
This paragraph has a comment above it.

This paragraph has a /*range */comment. It will be printed as "This paragraph has a comment."
```

## References

Each reference is defined in two places, using square brackets containing the reference number. The first is placed inline in the text, and the other is placed at the end of the chapter on its own line surrounded by blank lines. Each chapter should reset the reference number to 1. Generated documents may change the reference numbers if they are printed per page or per book.

```
This paragraph[1] contains a reference.

[1] A paragrph is a series of related sentences...
```

## Metadata

Metadata allows the author to provide more information to the press tool. Metadata attributes are placed with curly braces and must be placed at the start of a line. Available attributes are:

* Type - "Book" or "Article".
* Title - Title of the article or book. Articles without this will default to the top-level heading. Articles and books can fall back to the source text file name.
* Author - Author of the article or book.
* Authors - Each author must be separated by a comma and space ", ".
* Translator - Translator of the article or book.
* Translators - Each translator must be separated by a comma and space ", ".

```
{Type: Book}
{Title: Manifesto of the Communist Party}
{Authors: Karl Marx, Friedrich Engels}
{Translator: Samuel Moore}
```
