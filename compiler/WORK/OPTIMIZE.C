/*@A (C) 1992 Allen I. Holub                                                */
#include <stdlib.h>		/* <malloc.h> for UNIX */

typedef struct node
{
    char	 name[16];
    int          op;
    struct node  *left;
    struct node  *right;
}
node;

node *new()
{
    node *p;
    if( !(p = (node *) calloc( 1, sizeof(node) ) ) )
        exit(1);
    return p;
}
node    *build()
{
    char   buf[80];
    node   *stack[ 10 ];
    node   **sp = stack - 1;
    node   *p;

    while( gets(buf) )
    {
        switch( *buf )
        {
        default:  p = new();
                  strcpy( p->name, buf );
                  *++sp = p;
                  break;

        case '*':
        case '+':
                  p          = new() ;
                  p->right   = *sp-- ;
                  p->left    = *sp-- ;
                  p->op      = *buf  ;
                  p->name[0] = *buf  ;
                  *++sp      = p     ;
                  break;
        }
    }
    return *sp--;
}
trav( root )
struct node *root;
{
    static int tnum = 0;

    if( !root )
        return;

    if( !root->op )     /* leaf */
    {
        printf ( "t%d = %s\n", tnum, root->name );
        sprintf( root->name , "t%d", tnum );
        ++tnum;
    }
    else
    {
        trav( root->left  );

        if( root->left != root->right )    /* Always true      */
            trav( root->right );           /* unless optimized */

        printf("%s %c= %s\n", root->right->name,
                                root->op, root->left->name );
        strcpy( root->name,   root->right->name );
    }
}
optimize( root )   /* Simplified optimizer--eliminates common subexpressions. */
node    *root;
{
    char sig1[ 32 ];
    char sig2[ 32 ];

    if( root->right && root->left )
    {
        optimize( root->right );
        optimize( root->left  );

        *sig1 = *sig2 = '\0';
        makesig( root->right, sig1 );
        makesig( root->left , sig2 );

        if( strcmp( sig1, sig2 ) == 0 )   /* subtrees match */
            root->right = root->left ;
    }
}

makesig( root, str )
node    *root;
char    *str;
{
    if( !root )
        return;

    if( isdigit( *root->name )
	strcat( str, root->name );
    else
    {
	strcat( "<", root->name );
	strcat( str, root->name );
	strcat( ">", root->name );
    }
    makesig( root->left,  str );
    makesig( root->right, str );
}
