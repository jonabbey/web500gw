#
# Spanish/Castellano "Message File" for web500gw
# Spanish version: Javier.Massa@rediris.es
# Author: Frank.Richter@hrz.tu-chemnitz.de
# --> ANY HINTS WELCOME!
#
# $Id: web500gw.messages.es,v 1.3 2001/04/26 22:30:40 dekarl Exp $
#

# Local gateway admin - YOU - please change...
:ADMIN <A HREF="http://www.rediris.es/x500/coord/pidir.es.html">Administrador</A>

# HTML elements
# HTML start sequence
# Entry: %1 = RDN, %2 = UFN
:HTML_START_ENTRY <HTML><HEAD><TITLE>web500gw: %1</TITLE><META NAME="robots" CONTENT="noindex,nofollow"></HEAD>
<BODY BGCOLOR="#FFFFFF">

# If you want different start sequence when below HOMEDN:
:HTML_START_ENTRY_HOME <HTML><HEAD><TITLE>web500gw: %1</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF" TEXT="#000000" LINK="#006600" VLINK="#003300"
      ALINK="#00FF00">

# Search results: %1 = search filter, %2 = base DN
:HTML_START_SEARCH_RESULTS <HTML><HEAD>
<TITLE>web500gw: Resultado de la búsqueda: %1</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF">

# If you want different start sequence when searched below HOMEDN:
:HTML_START_SEARCH_RESULTS_HOME <HTML><HEAD>
<TITLE>web500gw: Resultado de la búsqueda: %1</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF" TEXT="#000000" LINK="#006600" VLINK="#003300"
      ALINK="#00FF00">

# Search results: %1 = search filter, %2 = base DN
:HTML_START_NO_SEARCH_RESULTS <HTML><HEAD>
<TITLE>web500gw: No se han encontrado resultados para la búsqueda: %1</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF">

# If you want different start sequence when searched below HOMEDN:
:HTML_START_NO_SEARCH_RESULTS_HOME <HTML><HEAD>
<TITLE>web500gw: No se han encontrado resultados para la búsqueda: %1</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF" TEXT="#000000" LINK="#006600" VLINK="#003300"
       ALINK="#00FF00">

# Misc: Bind/Modify: %1 = what (), %2$ = RDN, %3 = UFN
:HTML_START_MISC <HTML><HEAD><TITLE>web500gw: %1 - %2</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF">

# If you want different start sequence when below HOMEDN:
:HTML_START_MISC_HOME <HTML><HEAD><TITLE>web500gw: %1 - %2</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF" TEXT="#000000" LINK="#006600" VLINK="#003300"
      ALINK="#00FF00">

# Error: %1 = NO_SEARCH_RESULTS_TITLE | ERROR
:HTML_START_ERROR <HTML><HEAD><TITLE>web500gw: %1</TITLE></HEAD>
<BODY BGCOLOR="#FFFFFF"> 

# HTML closing sequence
:HTML_END   </BODY></HTML>

# The standard banner/trailer for every page
# specify a ' ' (space) if you don't want it
# %1 = current query, 
# %2 = other languages menu, %3 = other languages links
:BANNER <DIV ALIGN="center">[<A HREF="H?$lang=es">Ayuda</A>] 
%3
</DIV><HR>

:TRAILER <BR><BR><HR><DIV ALIGN="center"><SMALL>Powered by
<STRONG><A HREF="http://web500gw.sourceforge.net/" TARGET="_top">web500gw</A></STRONG>.
</SMALL></DIV>

# Banner/trailer for the pages below HomeDN
# %1 = current query
# %2 = other languages menu, %3 = other languages links
:HOMEBANNER <DIV ALIGN="center"><A HREF="H?$lang=es">[Ayuda]</A>
%3
<H1 ALIGN="center">Bienvenido al Servicio de Directorio!</H1>
</DIV><HR>

:HOMETRAILER <BR><BR><HR><DIV ALIGN="center"><SMALL>Powered by
<STRONG><A HREF="http://web500gw.sourceforge.net/" TARGET="_top">web500gw</A></STRONG>.
</SMALL></DIV>


# Banner/trailer for an error page
# %s = current query
:ERRORBANNER <DIV ALIGN="center"><A HREF="H?$lang=es">[Ayuda]</A> 
<A HREF="%s">[Otra vez]</A></DIV><HR>

# %1 = current query, %2 = ADMIN
:ERRORTRAILER <P><HR>
Por favor, redirija el error encontrado a su administrador: %2.


##################### Display of entries
# Entry name
# %1 = RDN, %2$S = UFN
:ENTRY_HEADER   <H1 ALIGN=center><A NAME="entry">%1</A></H1>

# HTML code used to display attributes
# %1 = URL, %2 = format, %3 = label
:IMG               <IMG SRC="%1" ALT="%2 %3">
:TABLE             <TABLE ALIGN="center" WIDTH="95%">
:TABLE_FORM        <TABLE ALIGN="center">
:TABLE_BORDER      <TABLE ALIGN="center" BORDER="1" WIDTH="95%">
:TABLE_BORDER_FORM <TABLE ALIGN="center" BORDER="1">
:HR                <P><HR NOSHADE><P>

# attribute name: %1 = name, %2 = empty or ":"
:ATTR_NAME     <STRONG>%1%2 </STRONG>
:ATTR_NAME_TD  <TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=top WIDTH="30%">

# Search: values only (valsonly)
# attribute name: %s
:ATTR_NAME_TH_SEARCH <TH ALIGN=center BGCOLOR="#e0e0e0">%s</TH>
# Search: attibutes + values
:ATTR_NAME_TD_SEARCH <TD ALIGN=left BGCOLOR="#e0e0e0">

# attribute value
:ATTR_VAL   %s
:ATTR_VAL_TD    <TD ALIGN=left>

# Linkaction (ldaptemplates.conf)
# %1 = URL, %2 = Label
# as form button
# :LINKACTION <FORM ACTION="%1"><INPUT TYPE="submit" VALUE="%2"></FORM>
# or simple link
:LINKACTION <A HREF="%1"><STRONG>%2</STRONG></A>

# objectclass in lists/tables
:OC_NAME         <STRONG><BIG>%s</BIG></STRONG>
:OC_NAME_TR      <TR><TD ALIGN=left VALIGN=top BGCOLOR="#e0e0e0">
:OC_NAME_TR_COLS <TR><TD ALIGN=center VALIGN=top COLSPAN=%d BGCOLOR="#e0e0e0">

# links to entries
# %1 = URL, %2 = UFN, %3 = DN
# %4 = string encoded UFN, %5 = string encoded DN
#  4/5  - for use in JavaScript - string literals
:HREF_LEAF 
<A HREF="%1" onMouseover="window.status='Ver %5';return true;">%2</A>
:HREF_NON_LEAF
<A HREF="%1" onMouseover="window.status='Ir a %5';return true;"><STRONG>%2</STRONG></A>

# aliases: %1 = URL, %2 = alias UFN, %3 = UFN, %4 = DN
# %5 = string encoded UFN, %6 = string encoded DN
#       - for use in JavaScript - string literals
:HREF_ALIAS_TO_LEAF
<A HREF="%1" onMouseover="window.status='Ver %6';return true;"><EM>%2</EM> -&gt; %3</A>
:HREF_ALIAS_TO_NONLEAF
<A HREF="%1" onMouseover="window.status='Ir a %6';return true;"><EM>%2</EM> -&gt; <STRONG>%3</STRONG></A>

# DNs without links (Flag nohrefdn):
# %1 = UFN, %2 = DN
:DN_LEAF          %1
:DN_NON_LEAF      %1

# Alias: %1 = alias UFN, %2 = UFN, %3 = DN
:DN_ALIAS_TO_LEAF     <EM>%1</EM> -&gt; %2
:DN_ALIAS_TO_NONLEAF  <EM>%1</EM> -&gt; <STRONG>%2</STRONG>

# A string to use when refering to the "X.500 root"
:ROOT			El Mundo
# boolean values true, false
:TRUE   TRUE
:FALSE  FALSE
:NO_HELPFILE  No puedo acceder al fichero de Ayuda!
:URL          URL
:URL_INFO     Informaciín&nbsp;


##################### Navigation and search forms
# Navigation and search as menu in table
# %1 = moveup action,     %2 = Navigation as <SELECT> menu
# %3 = search action URL, %4 = ONELEVEL | SUBTREE, 
# %5 = RDN (search base), %6 = UFN (search base), 
# %7 = scope (don't change) %8 = Link für erweiterte Suche
:NAV_MENU  <P>
<TABLE ALIGN="center" BORDER="0" 
  CELLPADDING="0" CELLSPACING="0" BGCOLOR="#000000" WIDTH="95%%">
<TR><TD>
<TABLE BORDER="0" WIDTH="100%%" CELLPADDING="5" CELLSPACING="1" BGCOLOR="#e0e0e0">
<TR ALIGN=center><TD VALIGN=middle>
  <FORM ACTION="%1"><!-- NAV_MENU -->
   <STRONG><INPUT TYPE=submit VALUE="Ir arriba a"></STRONG> <BR>
   %2
  </FORM>
 </TD>
 <TD ALIGN=center>
  <FORM ACTION="%3">
   <A NAME="search_form"><STRONG>%4 Buscar</STRONG> bajo</A> <STRONG>%5</STRONG>:<BR>
   <INPUT SIZE="15" NAME="%7">
   <STRONG><INPUT TYPE=submit VALUE="SEARCH"></STRONG>
   <INPUT TYPE=reset VALUE="Borrar"><BR>
     <A HREF="%8">Búsqueda avanzada</A> - <A HREF="H?$lang=es#search">Consejos de búsqueda</A>
  </FORM>
 </TD>
</TR>
</TABLE>
</TD></TR></TABLE>

# Navigation as list and search
# see NAV_MENU, but %2 = Navigation as <LI> entries
:NAV_LIST Ir hacia arriba a
 <UL>
    %2
 </UL>
 <FORM ACTION="%3">
   <A NAME="search_form">%4 buscar bajo</A> <STRONG>%5</STRONG>:
   <INPUT SIZE="15" NAME="%7">
   <STRONG><INPUT TYPE=submit VALUE="  Buscar  "></STRONG>
   <INPUT TYPE=reset VALUE="  Borrar  "><BR>
   <A HREF="%8">Búsqueda avanzada</A> - <A HREF="H?$lang=es#search">Consejos de búsqueda</A>
 </FORM>

# Navigation as simple links and search
# see NAV_MENU, but %2 = Navigation as strings with links
:NAV_SMALL Ir hacia arriba a: <STRONG>%2</STRONG><BR>
 <FORM ACTION="%3">
   <A NAME="search_form">%4 buscar bajo</A> <STRONG>%5</STRONG>:
   <INPUT SIZE="15" NAME="%7">
   <STRONG><INPUT TYPE=submit VALUE="  Buscar  "></STRONG>
   <INPUT TYPE=reset VALUE="  Borrar  "><BR>
   <A HREF="%8">Búsqueda avanzada</A> - <A HREF="H?$lang=es#search">Consejos de búsqueda</A>
 </FORM>

:ONELEVEL		Un nivel
:SUBTREE		Recursiva

# Enhanced search formular
# %1 = search action URL, %2 = current search base 
# %3 = current search base (friendly)
# %4 = current country,   %5 = current org
# form NAMEs: base = search base
#             filtertemplate = a template for search filter
#             objectclass = objectclass to search for
#             searchattr = search value in attribute
#             query = value to seach for
#             match = matching template
#             attr = show attributes, flags = flags
#             args=val=arg&...&endargs = combination of above variables
:EXT_SEARCH_FORM_TITLE Búsqueda avanzada
:EXT_SEARCH_FORM
<H1 ALIGN=center>Búsqueda avanzada</H1>
<FORM ACTION="%1" METHOD=POST>
<INPUT TYPE="hidden" NAME="filtertemplate" VALUE="(&(objectclass=%%v1)(%%v2))">
<TABLE BORDER="0" CELLPADDING="1" CELLSPACING="0" BGCOLOR="#000000"ALIGN="center">
<TR><TD>
<TABLE WIDTH="100%%" BGCOLOR="#FFFFFF" CELLPADDING="5" BORDER="0" CELLSPACING="0">
<TR><TH ALIGN="right" BGCOLOR="#e0e0e0">Buscar bajo:</TH>
    <TD ALIGN="left" COLSPAN="2"><STRONG>
   <SELECT NAME="args">
   <OPTION VALUE="base=%2&endargs">%3
   <OPTION VALUE="base=o=rediris,c=ES&endargs">RedIRIS, España
   <OPTION VALUE="base=o=Technische Universitaet Chemnitz,c=DE&endargs">TU Chemnitz
   <OPTION VALUE="ldapserver=ldap.bigfoot.com&base=&flags=nodn&endargs">Bigfoot
   <OPTION VALUE="ldapserver=ldap.four11.com&base=&flags=nodn&endargs">Four11
   <OPTION VALUE="ldapserver=ldap.switchboard.com&base=c=US&flags=nodn&endargs">Switchboard 
   <OPTION VALUE="ldapserver=ldap.infospace.com&base=c=US&flags=nodn&endargs">Infospace
   <OPTION VALUE="filtertemplate=(%%25v2)&ldapserver=ldap.whowhere.com&base=&attr=cn&flags=nodn&endargs">Whowhere
   </SELECT></STRONG>
  </TD>
</TR>
<TR><TH ALIGN="right" BGCOLOR="#e0e0e0">for:</TH>
    <TD ALIGN="left" COLSPAN=2>
       <SELECT NAME="objectclass">
       <OPTION VALUE="person">Persona
       <OPTION VALUE="organizationalUnit">Departmento
       <OPTION VALUE="room">Despacho
       <OPTION VALUE="*">Cualquiera
       </SELECT>with
       <SELECT NAME="searchattr">
       <OPTION VALUE="cn">Nombre
       <OPTION VALUE="sn">Apellido
       <OPTION VALUE="mail">E-Mail
       <OPTION VALUE="description">Descripción
       <OPTION VALUE="ou">Departmento
       </SELECT>
       <SELECT NAME="match">
       <OPTION VALUE="%%a=*%%v*">contiene:
       <OPTION VALUE="!(%%a=*%%v*)">no contiene:
       <OPTION VALUE="%%a~=%%v">suena como:
       <OPTION VALUE="%%a=%%v">es:
       </SELECT>
       <INPUT NAME="query" SIZE=45>
</TD></TR>
<TR><TH ALIGN="right" BGCOLOR="#e0e0e0">Muestra<BR>Atributos:</TH>
    <TD ALIGN="left">
       <SELECT NAME="attr" SIZE="5" MULTIPLE>
       <OPTION VALUE="mail" SELECTED>E-Mail
       <OPTION VALUE="telephoneNumber" SELECTED>Teléfono
       <OPTION VALUE="vcard">vCard - Tarjeta de visita
       <OPTION VALUE="postalAddress">Dirección
       <OPTION VALUE="labeledURI">URL
       <OPTION VALUE="jpegPhoto">Fotografía
       <OPTION VALUE="description">Descripción
       </SELECT></TD>
    <TD ALIGN="left">as 
      <SELECT NAME="flags">
      <OPTION VALUE="btable" SELECTED>Tabla (con borde)
      <OPTION VALUE="table">Tabla
      <OPTION VALUE="list">Lista
      <OPTION VALUE="oneline">Compacta
      </SELECT>
      <P>
      <INPUT TYPE="checkbox" NAME="flags" VALUE="valsonly" CHECKED> Sólo valores
      <INPUT TYPE="checkbox" NAME="flags" VALUE="entryonly" CHECKED> Sólo resultados
</TD></TR>
<TR>
  <TD COLSPAN=3 BGCOLOR="#e0e0e0" ALIGN=center>
  <STRONG><INPUT TYPE=submit VALUE="  Buscar  "></STRONG>
  <INPUT TYPE=reset VALUE="  Borrar  ">
</TD></TR>
</TABLE></TD></TR></TABLE>
</FORM>


##################### Search results
:NO_LIST_ACCESS
No tenemos acceso para realizar una búsqueda a nivel global desde aquí.
Si conoce a quien está buscando, intente escoger la opción de
<STRONG><A HREF="#search_form">Búsqueda</A></STRONG> apropiada
y especifique el nombre de la entrada que desea.<p>
:NOTHING_FOUND_READORSEARCH	No se encontró nada por debajo de esta entrada!
Puede leer la entrada o intentar otra búsqueda ...

# Search results
# %1 = search type (ONELEVEL | SUBTREE), %2 = search filter,
# %3 = "human readable" search filter, %4 = base DN
# %5 = # of matches, %6 = ENTRY || ENTRIES, %7 = filter description
# %8 = empty or LDAP_PARTIAL_RESULTS
:SEARCH_RESULTS		<P>
<TABLE ALIGN="center" BORDER="0"
  CELLPADDING="0" CELLSPACING="0" BGCOLOR="#000000" WIDTH="95%%">
<TR><TD>
<TABLE BORDER="0" WIDTH="100%%" CELLPADDING="5" CELLSPACING="1">
<TR><TD ALIGN="RIGHT" BGCOLOR="#E0E0E0"><A NAME="search">Resultados de la búsqueda (%1)</A> for</TD>
    <TD BGCOLOR="#FFFFFF"><STRONG>%3</STRONG></TD></TR>
<TR><TD ALIGN="RIGHT" BGCOLOR="#E0E0E0">bajo</TD>
    <TD BGCOLOR="#FFFFFF"><STRONG>%4</STRONG></TD></TR>
<TR BGCOLOR="#E0E0E0"><TD ALIGN="center" COLSPAN="2"><EM>Encontradas %5 %6
         (<STRONG>%7 %8</STRONG>).</EM></TD></TR>
</TABLE></TD></TR>
</TABLE>

:ENTRY		entrada
:ENTRIES	entradas
:UFN        UFN
:COMPLEX_FILTER complex search filter

# No search result
# %1 = search type (ONELEVEL | SUBTREE), %2 = search filter,
# %3 = "human readable" search filter, %4 = base DN
:NO_SEARCH_RESULTS     <TABLE ALIGN="center">
<TR><TD ALIGN="RIGHT"><A NAME="search">No se encontró nada</A> para</TD>
    <TD><STRONG>%3</STRONG></TD></TR>
<TR><TD ALIGN="RIGHT">bajo</TD>
    <TD><STRONG>%4</STRONG></TD></TR>
</TABLE><HR>

# Error in search specification
:MISSING_FILTER         Esperando filtro
:MISSING_SCOPE          Esperando el ámbito de búsqueda
:MISSING_SEARCHVALUE    Esperando la cadena a buscar
:SPECIFY_SEARCHVALUE    Por favor especifique una cadena a buscar
:SPECIFY_SUBTREE_SEARCHVALUE	Por favor especifique una cadena a buscar o un '*? para
realizar una búsqueda con comodines.

# Redirect: exactly one search result
# - for old browsers - new browsers will connect automatically
# %1 = HREF, %2 = DN
:REDIRECT <DL><DT>Encontrada una : <DD><A HREF="%1">%2</A></DL>


##################### Modification
# Bind
# for <TITLE>
:ASK_FOR_BIND       Pregunta por información del usuario

# Bind form
# %1 = RDN, %2 = UFN, %3 = ACTION URL, 
# %4 = Bind DNs as SELECT OPTIONS or INPUT field
:BIND_FORM       <DIV ALIGN="center">Para modificar la entrada de
<H2 ALIGN="center">%1</H2>
ha de conectarse al directorio.
</DIV>
<FORM METHOD=POST ACTION="%3">
<TABLE ALIGN="center">
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
        <STRONG>Conectando como usuario:</STRONG></TD>
    <TD>%4</TD>
</TR>
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
        <STRONG>Clave de este usuario:</STRONG></TD>
    <TD><INPUT TYPE="password" NAME="userPassword"></TD></TR>
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=top>
        <STRONG>Cambio:</STRONG></TD>
    <TD>
     <INPUT TYPE="radio" NAME="attrs" VALUE="" CHECKED> Entrada
     &nbsp;&nbsp;&nbsp;&nbsp;
     <INPUT TYPE="radio" NAME="attrs" VALUE="userPassword"> Clave
</TD></TR>
</TABLE>
<P><HR>
<DIV ALIGN="center">
<STRONG><INPUT TYPE="submit" VALUE=" Conexión para Modificar "></STRONG>
<INPUT TYPE="reset" VALUE="Borrar datos">
</DIV></FORM>

:MISSING_PASSWORD	Clave no introducida
:UNKNOWN_ARGUMENT	Argumento desconocido
:NULL_PASSWORD      Por favor introduzca una clave
:PASSWORD_VERIFY    No puedo verificar la clave
:PASSWORD_MISMATCH  Error en la clave! Escribió dos claves diferentes ... 
:DELETED	borrado
:ADDED		añadido
:REPLACED	reemplazado
:CHANGE		cambio
:CHANGES	cambios

:MISSING_DN 	    Esperando DN
:UNKNOWN_QUERY      Consulta desconocida
:MODIFY_RESULTS     Resultados de modificación

# Modification
:MODIFY			Modificar
# %1 = RDN, %2 = UFN
:EXPLAIN_MODIFY <DIV ALIGN="center">Modificar
<H2 ALIGN=center>%1</H2>
</DIV>

:HELP_MODIFY <DIV ALIGN="center">Pulse sobre el nombre del atributo para obtener
ayuda sobre su formato.</DIV>

# Help for attributes
# %1 = friendly attribute name, %2 = attribute name, 
# %3 empty or  ATTR_REQUIRED
:ATTR_HELP <A HREF="HA#%2">%1</A>%3
:ATTR_REQUIRED <BR>debe ser especificado!

# Changing password
:CHANGE_PASSWORD <DIV ALIGN="center"><STRONG>Cambio de su clave en el Directorio</STRONG><P>
Por favor, introduzca su clave dos veces.
</DIV>
<TABLE ALIGN="center">
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
      <STRONG>Nueva clave:</STRONG></TD>
    <TD><INPUT TYPE="password" NAME="userPassword"></TD></TR>
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
      <STRONG>Otra vez:</STRONG></TD>
    <TD><INPUT TYPE="password" NAME="verify_userPassword"></TD></TR>
</TABLE>

:MODIFY_ACTION      <P><HR>
<DIV ALIGN="center">
<STRONG><INPUT TYPE="submit" VALUE="  Modificar  "></STRONG>
<INPUT TYPE="reset" VALUE="  Borrar valores  ">
</DIV>

# %1 = RDN, %2 = UFN of the changed entry
:MODIFY_RESULTS_FOR	<DIV ALIGN="center">Modifcar resultados de
<H2  ALIGN=center>%1</H2></DIV>
<TABLE ALIGN="center">
<TR><TD COLSPAN=2 BGCOLOR="#e0e0e0"><STRONG><BIG>Cambios</BIG></STRONG></TD></TR>

# Modification successful: %1 = attribute, %2 = new value, %3 = action
:MODIFY_REPORT  <TR>
   <TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=top WIDTH="30%">
      <STRONG>%1 <EM>%3</EM></STRONG></TD>
   <TD>%2</TD></TR>

# %1 = no of successful changes, %2 = CHANGE/CHANGES
:MODIFY_SUMMARY <TR><TD COLSPAN=2 BGCOLOR="#e0e0e0"><STRONG><BIG>%1 %2
realizado satisfactoriamente!</BIG></STRONG></TD></TR>

# Actions after modification
# %1 = URL to read, %2 = URL modify again, %3 = bind-dn, %4 = passwd
:AFTER_MODIFY   <TR><TD COLSPAN=2>
  <FORM ACTION="%2" METHOD=POST><STRONG>
  <A HREF="%1">Leer la entrada modificada</A>
  <INPUT TYPE=hidden NAME="dn" VALUE="%3">
  <INPUT TYPE=hidden NAME="userPassword" VALUE="%4">
  <INPUT TYPE=submit VALUE="Modificar otra vez"></STRONG></FORM>
  </TD></TR>
</TABLE>

# Error during modification:
# %1 = error code, %2 = error string, %3 = additional info
:MODIFY_ERROR   <TR>
  <TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=top WIDTH="30%">
    <STRONG><FONT COLOR="red">Error durante la operación de modificación:</FONT></TD>
  <TD>Código de error <EM> %1: %2.</EM><BR>%3</TD></TR>
</TABLE>

:NO_CHANGES <TR><TD COLSPAN=2><STRONG>No hay cambios!</STRONG></TD></TR>


##################### Add - new entry
# Bind for add form
# %1 = base RDN, %2 = base UFN, %3 = ACTION URL, 
# %4 = Bind DN as SELECT OPTIONS or INPUT field
:BIND_ADD_FORM       <DIV ALIGN="center">Para añadir una entrada bajo
<H2 ALIGN="center">%1</H2>
debe conectarse al Directorio como el administrador.
</DIV>
<FORM METHOD=POST ACTION="%3">
<TABLE ALIGN="center">
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
        <STRONG>Administrador:</STRONG></TD>
    <TD>%4</TD>
</TR>
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
        <STRONG>Clave:</STRONG></TD>
    <TD><INPUT TYPE="password" NAME="userPassword"></TD></TR>
</TABLE>
<P><HR>
<DIV ALIGN="center">
<STRONG><INPUT TYPE="submit" VALUE="  Comience autentificación  "></STRONG>
<INPUT TYPE="reset" VALUE="  Borrar valores  ">
</DIV></FORM>

:ADD    Añadir
# %1 = Base RDN, %2 = Base UFN
:EXPLAIN_ADD <DIV ALIGN="center">Añadir una entrada bajo
<H2 ALIGN=center>%2</H2></DIV>

:ADD_ACTION      <P><HR>
<DIV ALIGN="center">
<STRONG><INPUT TYPE="submit" VALUE="  Añadir  "></STRONG>
<INPUT TYPE="reset" VALUE=" Borrar valores ">
</DIV>

:ADD_RESULTS        Añadir una nueva entrada

# %1 = RDN, %2 = UFN of the added entries
:ADD_RESULTS_FOR    <DIV ALIGN="center">Añadido
<H2 ALIGN=center>%1</H2></DIV>

# Error during add
# %1 = RDN, %2 = UFN, %3 = error code, %4 = error description, 
# %5 = info
:ADD_ERROR <P>Error <EM>%3: %4</EM> - %5<P>
<STRONG>No se añadió ninguna entrada!</STRONG>

## %1 = URL to read, %2 = URL to modify, %3 = bind-dn, %4 = passwd
# Add ok
# %1 = RDN, %2 = UFN, %3 = URL z. Lesen
:ADD_OK  <STRONG>%2 añadidas satisfactoriamente!</STRONG><P>
         <STRONG><A HREF="%3">Leer la nueva entrada</A></STRONG>

:ADD_OC_NOT_ADDABLE     Entries of this objectclass are not addable!
:ADD_NO_RDN_DEFINED     No RDN attribute defined in template!
:ADD_NO_RDN             No RDN attribute defined!
:ADD_MISSING_REQ_VALUE  A value for the attribute "%s" is required!

##################### Delete entry
# Bind for delete form
# %1 = RDN, %2 = RDN with attr, %3 = base UFN, %4 = ACTION URL, 
# %5 = Bind DN as SELECT OPTIONS or INPUT field
:BIND_DELETE_FORM       <DIV ALIGN="center">To delete the entry
<H2 ALIGN="center">%1</H2>
you have to authenticate to the Directory.
</DIV>
<FORM METHOD=POST ACTION="%4">
<TABLE ALIGN="center">
<TR><TD ALIGN=right BGCOLOR="#FFFFCC" VALIGN=middle>
        <STRONG>Authenticate as user:</STRONG></TD>
    <TD>%5</TD>
</TR>
<TR><TD ALIGN=right BGCOLOR="#FFFFCC" VALIGN=middle>
        <STRONG>Password:</STRONG></TD>
    <TD><INPUT TYPE="password" NAME="userPassword"></TD></TR>
</TABLE>
<P><HR>
<DIV ALIGN="center">
<STRONG><INPUT TYPE="submit" VALUE="Yes, DELETE this entry"></STRONG>
<INPUT TYPE="reset" VALUE="RESET values">
</DIV></FORM>

:DELETE_RESULTS     Delete an entry

# %1 = RDN, %2 = UFN of the deleted entry
:DELETE_RESULTS_FOR <DIV ALIGN="center">Removal of
<H2 ALIGN=center>%1</H2></DIV>

# Error 
# %1 = RDN, %2 = UFN, %3 = error code, %4 = description, %5 = info
:DELETE_ERROR <P>Error <EM>%3: %4</EM> - %5<P>
<STRONG>The entry was not deleted!</STRONG>

# Delete ok
# %1 = RDN, %2 = UFN
:DELETE_OK  <DL><DT>The entry
<DD><STRONG>%2</STRONG>
<DT>was <STRONG>deleted</STRONG> successfully!
</DL>


##################### Rename entry (modify_rdn)
# Bind for MODRDN form
# %1 = old RDN, %2 = old RDN with attr, %3 = old UFN, %4 = ACTION URL, 
# %5 = Bind DN as SELECT OPTIONS or INPUT field
:BIND_MODRDN_FORM       <DIV ALIGN="center">To rename the entry
<H2 ALIGN="center">%1</H2>
you have to authenticate to the Directory.
</DIV>
<FORM METHOD=POST ACTION="%4">
<TABLE ALIGN="center">
<TR><TD ALIGN=right BGCOLOR="#FFFFCC" VALIGN=middle>
        <STRONG>Authenticate as user:</STRONG></TD>
    <TD>%5</TD>
</TR>
<TR><TD ALIGN=right BGCOLOR="#FFFFCC" VALIGN=middle>
        <STRONG>Password:</STRONG></TD>
    <TD><INPUT TYPE="password" NAME="userPassword"></TD></TR>
<TR><TD><BR><BR></TD><TD></TD></TR>
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
        <STRONG>Old name:</STRONG></TD>
    <TD>%2
    </TD></TR>
<TR><TD ALIGN=right BGCOLOR="#e0e0e0" VALIGN=middle>
        <STRONG>New name:</STRONG></TD>
        <TD><INPUT NAME="newrdn"><BR>
         </TD></TR>
<TR><TD><BR><TD>
        <INPUT TYPE="checkbox" NAME="deleteoldrdn" VALUE="1" CHECKED>
        Remove old name form the entry?
    </TD></TR>
</TABLE>
<P><HR>
<DIV ALIGN="center">
<STRONG><INPUT TYPE="submit" VALUE="Yes, RENAME this entry"></STRONG>
<INPUT TYPE="reset" VALUE="RESET values">
</DIV></FORM>

:MODRDN_NO_NEWRDN   No new name given!
:MODRDN_RESULTS     Rename an entry

# %1 = old RDN, %2 = old UFN, %3 = new RDN, %4 = new UFN
:MODRDN_RESULTS_FOR <DIV ALIGN="center">Renaming
<H2 ALIGN=center>%1</H2> to
<H2 ALIGN=center>%3</H2></DIV>

# modrdn error
# %1 = old RDN, %2 = old UFN, %3 = new RDN, %4 = new UFN,
# %5 = error code, %6 = description, %6 = info
:MODRDN_ERROR <P>Error <EM>%5: %6</EM> - %7<P>
<STRONG>The entry was not renamed!</STRONG>

# modrdn ok
# %1 = old RDN, %2 = old UFN, %3 = new RDN, %4 = new UFN
# %5 = URL for the new entry
:MODRDN_OK  <DL>
<DT>The entry <DD>%2
<DT>was renamed to
<DD><STRONG><A HREF="%5">%4</A></STRONG>
</DL>
##################### Errors and error descriptions
# %d = # of entries found
:SIZELIMIT_0 <P>
La pregunta que ha especificado no es lo suficiente concreta y ha
provocado que se devuelvan muchas entradas. Se ha excedido el número %d
permitido. Si no encontró la entrada que buscaba intente realizar 
una consulta algo más específica, por ejemplo que contenga el nombre y
el primer apellido.
:SIZELIMIT_1 <P>
Solo se han devuelto %d entradas, porque se excedió el límite permitido.
No tenemos un mecanismo automático para saltarnos este límite, pero si
conoce a quien está buscando, escoja otra opción de búsqueda de entre
las mostradas arriba y especifique el nombre de la entrada que busca.
:TIMELIMIT <P>
Ha ocurrido un timeout mientras se buscaba en el Directorio, y no
se han podido devolver datos. Por favor inténtelo más tarde.

# HTTP responses for explanation in error pages
:DOCUMENT_FOLLOWS   Correcto
:REDIRECT           Encontrado
:NOT_MODIFIED       No modificado
:BAD_REQUEST		Mala petición
:AUTH_REQUIRED      Se requiere autorización
:FORBIDDEN          Olvidado
:NOT_FOUND          No encontrado
:SERVER_ERROR       Error del servidor
:NOT_IMPLEMENTED    No implementado
:UNKNOWN_ERROR      Error desconocido
:ERROR              Error

# Error explanation:
# %1 = HTTP-error, %2 = LDAP code, %3 = LDAP error string
# %4 = ERR_MATCHED %5 = more information
:ERROR_OCCURRED		<H2>Fallo en el Directorio: %1</H2>
Ha ocurrido un error en esta operación del Directorio.<P>
El código de error fue: <EM>%2</EM>:
<EM>%3.</EM><BR>
%4
%5


# Matched for LDAP_NO_SUCH_OBJECT: %1 = matched URL, %2 = matched DN
:ERR_MATCHED    <P>Coincidencia: <A HREF="%1">%2</A><P>

:NOT_SUPPORTED   <STRONG>Esta característica no está soportada por este servidor.</STRONG>
:TEMPLATE_MISSING   Template not found!
:NO_BIND_DN         No DN for bind!

# ACCESS:
:ACCESS_NOTHING    <STRONG>Su dominio no tiene permiso de acceso a este servicio! 
</STRONG>
:ACCESS_READONLY   <STRONG>Sólo tiene permiso de lectura - no puede realizar modificaciones!
</STRONG>
:ACCESS_COMMON     <STRONG>Esta acción no está permitida para usted</STRONG>

#
# The LDAP errors for our own ldap_err2string routine
#
:LDAP_SUCCESS		Éxito
:LDAP_OPERATIONS_ERROR		Error de la operación
:LDAP_PROTOCOL_ERROR		Error del protocolo
:LDAP_TIMELIMIT_EXCEEDED		Excedido límite de tiempo
:LDAP_SIZELIMIT_EXCEEDED		Excedido límite de tamaño
:LDAP_COMPARE_FALSE		Comparación falsa
:LDAP_COMPARE_TRUE		Comparación verdadera
:LDAP_STRONG_AUTH_NOT_SUPPORTED		Autentificación fuerte no soportada
:LDAP_STRONG_AUTH_REQUIRED		Autentificación fuerte requerida
:LDAP_PARTIAL_RESULTS       Resultados parciales
:LDAP_NO_SUCH_ATTRIBUTE		No existe ese atributo
:LDAP_UNDEFINED_TYPE		Tipo de atributo no definido
:LDAP_INAPPROPRIATE_MATCHING		Coincidencia inapropiada
:LDAP_CONSTRAINT_VIOLATION		Violación de restricción
:LDAP_TYPE_OR_VALUE_EXISTS		Existe el tipo o el valor
:LDAP_INVALID_SYNTAX		Sintaxis inválida
:LDAP_NO_SUCH_OBJECT		No existe ese objeto
:LDAP_ALIAS_PROBLEM		Problema con un objeto de tipo alias
:LDAP_INVALID_DN_SYNTAX		Sintaxis inválida del DN
:LDAP_IS_LEAF		El objeto es una hoja
:LDAP_ALIAS_DEREF_PROBLEM		Problema de referencia de alias
:LDAP_INAPPROPRIATE_AUTH		Autentificación inapropiada
:LDAP_INVALID_CREDENTIALS		Credenciales inválidas
:LDAP_INSUFFICIENT_ACCESS		Acceso insuficiente
:LDAP_BUSY		El DSA está ocupado
:LDAP_UNAVAILABLE		El DSA no está accesible
:LDAP_UNWILLING_TO_PERFORM		El DSA no puede hacerlo
:LDAP_LOOP_DETECT		Detectado un bucle
:LDAP_NAMING_VIOLATION		Violación de nombre
:LDAP_OBJECT_CLASS_VIOLATION		Violación de clase de objeto
:LDAP_NOT_ALLOWED_ON_NONLEAF		Operación no permitida en una hoja
:LDAP_NOT_ALLOWED_ON_RDN		Operación no permitida en un RDN
:LDAP_ALREADY_EXISTS		Ya existe
:LDAP_NO_OBJECT_CLASS_MODS		No puedo modificar el tipo de objeto
:LDAP_RESULTS_TOO_LARGE		Resultado demasiado grande
:LDAP_OTHER		Error desconocido
:LDAP_SERVER_DOWN		No puedo contactar con el servidor LDAP
:LDAP_LOCAL_ERROR		Error local
:LDAP_ENCODING_ERROR		Error de codificación
:LDAP_DECODING_ERROR		Error en la decodificación
:LDAP_TIMEOUT		Timed out
:LDAP_AUTH_UNKNOWN		Método de autenticación desconocido
:LDAP_FILTER_ERROR		Filtro de búsqueda incorrecto
:LDAP_USER_CANCELLED		El ususario ha cancelado la operación
:LDAP_PARAM_ERROR		Parámetro incorrecto a una rutina LDAP
:LDAP_NO_MEMORY		Fuera de memoria


# Web500gw/LDAP monitor
# %1 = version, %2 = compile time, %3 = LDAP version, %4 = copyright,
# %5 = ADMIN, %6 = basedn-URL, %7 = basedn-UFN, %8 = LDAP server,
# %9d = LDAP port
# %10 = start time, %11 = current time, %12 = total connections,
# %13 = active connections

:MONITOR_TITLE  Monitor
:MONITOR    <H1>%1</H1>
<EM>%4</EM><P>
<STRONG>Compilado en %2 - usando %3</STRONG><HR>
<TABLE>
<TR><TD>Admin</TD><TD%5</TD></TR>
<TR><TD>"HOME"</TD><TD><A HREF="%6">%7</A></TD></TR>
<TR><TD>Servidor LDAP + puerto</TD><TD>ldap://%8:%9/</TD></TR>
<TR><TD>Hora de comienzo</TD><TD>%10</TD></TR>
<TR><TD>Hora actual</TD><TD>%11</TD></TR>
<TR><TD>Total de peticiones</TD><TD>%12</TD></TR>
<TR><TD>Petición actual</TD><TD>%13</TD></TR>
</TABLE>
<HR>

