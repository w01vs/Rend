# Info

## Grammar

$$
\begin{align*}
    \text{<Program>} &\to \text{<Statements>} \\
    \text{<Statements>} &\to
    \begin{cases}
        \text{<Statement> <Statements>} \\
        \epsilon
    \end{cases} \\
    \text{<Statement>} &\to
    \begin{cases}
        \text{KEYWORD('return') <Expression> SYMBOL(';')} \\
        \text{TYPE IDENTIFIER SYMBOL('=') <Expression> SYMBOL(';')} \\
        \text{<Scope>} \\
        \text{<If>} \\
        \text{KEYWORD('while') SYMBOL('(') <Expression> SYMBOL(')') <Scope>} \\
        \text{IDENTIFIER SYMBOL('=') <Expression> SYMBOL(';')} \\
        \text{KEYWORD('struct') IDENTIFIER SYMBOL('\{') <StructBody> SYMBOL('\}')} \\
        \text{TYPE IDENTIFIER SYMBOL(';')}
    \end{cases} \\
    \text{<Scope>} &\to \text{SYMBOL('\{') <Statements> SYMBOL('\}')} \\
    \text{<If>} &\to
    \begin{cases}
        \text{KEYWORD('if') SYMBOL('(') <Expression> SYMBOL(')') <Scope> <Else>} \\
    \end{cases} \\
    \text{<Else>} &\to
    \begin{cases}
        \text{KEYWORD('else') <Scope>} \\
        \text{KEYWORD('else') <If>} \\
        \epsilon \\
    \end{cases} \\
    \text{<Expression>} &\to
    \begin{cases}
        \text{IDENTIFIER OP EXPRESSION} \\
        \text{INTEGER OP EXPRESSION} \\
        \text{LOGIC\_VALUE OP EXPRESSION} \\
    \end{cases} \\
    % \text{<StructBody>} &\to
    % \begin{cases}
    %     \text{TYPE IDENTIFIER SYMBOL(';') <StructBody>} \\
    %     \text{TYPE IDENTIFIER SYMBOL('=') <Expression> <StructBody>}
    %     \epsilon \\
    % \end{cases} \\
\end{align*}
$$

## Parsed Symbols

$$
\begin{array}{r l}
\text{Builtin types:} & \text{int} \\
& \text{bool} \\
\hline
\text{Operators:} & \text{+} \\
& \text{-} \\
& \text{*} \\
& \text{/} \\
& \text{\%} \\
& \text{\&\&} \\
& \text{||} \\
& \text{!} \\
& \text{==} \\
& \text{!=} \\
& \text{>} \\
& \text{<} \\
& \text{>=} \\
& \text{<=} \\
\hline
\text{Symbols:} & \text{;} \\
& \text{=} \\
& \text{(} \\
& \text{)} \\
& \text{\{} \\
& \text{\}} \\
& \text{,} \\
\hline
\text{Keywords:} & \text{if} \\
& \text{else} \\
& \text{while} \\
& \text{continue} \\
& \text{break} \\
& \text{return} \\
& \text{struct} \\
\hline
\text{Identifiers:} & \text{regex/[a-zA-Z][a-zA-Z0-9]*} \\
\hline
\text{Integers:} & \text{\textbackslash d+} \\
\hline
\text{Ignored whitespace:} & \text{\textbackslash r} \\
& \text{\textbackslash n} \\
& \text{\textbackslash s+} \\
\end{array}
$$
