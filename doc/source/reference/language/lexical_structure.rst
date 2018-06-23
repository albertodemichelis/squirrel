.. _lexical_structure:


=================
Lexical Structure
=================

.. index:: single: lexical structure

-----------
Identifiers
-----------

.. index:: single: identifiers

Identifiers must start with an alphabetic character or an underscore followed by any number
of alphabetic characters, underscores or digits (0-9). Squirrel is a case-sensitive language, "foo" and "Foo" are
treated as two different identifiers.

-----------
Keywords
-----------

.. index:: single: keywords

The following words are reserved and cannot be used as identifiers:

+------------+------------+-----------+------------+------------+-------------+
| base       | break      | case      | catch      | class      | clone       |
+------------+------------+-----------+------------+------------+-------------+
| continue   | const      | default   | delete     | else       | enum        |
+------------+------------+-----------+------------+------------+-------------+
| extends    | for        | foreach   | function   | if         | in          |
+------------+------------+-----------+------------+------------+-------------+
| local      | null       | resume    | return     | switch     | this        |
+------------+------------+-----------+------------+------------+-------------+
| throw      | try        | typeof    | while      | yield      | constructor |
+------------+------------+-----------+------------+------------+-------------+
| instanceof | true       | false     | static     | __LINE__   | __FILE__    |
+------------+------------+-----------+------------+------------+-------------+

Keywords are covered in detail later in this document.

-----------
Operators
-----------

.. index:: single: operators

Squirrel recognizes the following operators:

+----------+----------+----------+----------+----------+----------+----------+----------+
| ``!``    | ``!=``   | ``||``   | ``==``   | ``&&``   | ``>=``   | ``<=``   | ``>``    |
+----------+----------+----------+----------+----------+----------+----------+----------+
| ``<=>``  | ``+``    | ``+=``   | ``-``    | ``-=``   | ``/``    | ``/=``   | ``*``    |
+----------+----------+----------+----------+----------+----------+----------+----------+
| ``*=``   | ``%``    | ``%=``   | ``++``   | ``--``   | ``<-``   | ``=``    | ``&``    |
+----------+----------+----------+----------+----------+----------+----------+----------+
| ``^``    | ``|``    | ``~``    | ``>>``   | ``<<``   | ``>>>``  |          |          |
+----------+----------+----------+----------+----------+----------+----------+----------+

------------
Other tokens
------------

.. index::
    single: delimiters
    single: other tokens

Other significant tokens are:

+----------+----------+----------+----------+----------+----------+
| ``{``    | ``}``    | ``[``    | ``]``    | ``.``    | ``:``    |
+----------+----------+----------+----------+----------+----------+
| ``::``   | ``'``    | ``;``    | ``"``    | ``@"``   |          |
+----------+----------+----------+----------+----------+----------+

-----------
Literals
-----------

.. index::
    single: literals
    single: string literals
    single: numeric literals

Squirrel accepts integer numbers, floating point numbers and string literals.

+-------------------------------+------------------------------------------+
| ``34``                        | Integer number(base 10)                  |
+-------------------------------+------------------------------------------+
| ``0xFF00A120``                | Integer number(base 16)                  |
+-------------------------------+------------------------------------------+
| ``0753``                      | Integer number(base 8)                   |
+-------------------------------+------------------------------------------+
| ``'a'``                       | Integer number                           |
+-------------------------------+------------------------------------------+
| ``1.52``                      | Floating point number                    |
+-------------------------------+------------------------------------------+
| ``1.e2``                      | Floating point number                    |
+-------------------------------+------------------------------------------+
| ``1.e-2``                     | Floating point number                    |
+-------------------------------+------------------------------------------+
| ``"I'm a string"``            | String                                   |
+-------------------------------+------------------------------------------+
| ``@"I'm a verbatim string"``  | String                                   |
+-------------------------------+------------------------------------------+
| ``@" I'm a``                  |                                          |
| ``multiline verbatim string`` |                                          |
| ``"``                         | String                                   |
+-------------------------------+------------------------------------------+

Pesudo BNF

.. productionlist::
    IntegerLiteral : [1-9][0-9]* | '0x' [0-9A-Fa-f]+ | ''' [.]+ ''' | 0[0-7]+
    FloatLiteral : [0-9]+ '.' [0-9]+
    FloatLiteral : [0-9]+ '.' 'e'|'E' '+'|'-' [0-9]+
    StringLiteral: '"'[.]* '"'
    VerbatimStringLiteral: '@''"'[.]* '"'

-----------
Comments
-----------

.. index:: single: comments

A comment is an annotation that is ignored by the compiler. Squirrel supports the traditional C-style comments.
Block comments are delimited by ``/*`` and ``*/`` and can span multiple lines.::

    /*
    This is a multiline comment,
    these lines will be ignored.
    */

Line comments are delimited by two slashes ``//``.::

    //This is a line comment.

You can also use the number sign ``#`` as an alternative delimiter for line comments.
It allows you to start your Squirrel script with a shebang.
