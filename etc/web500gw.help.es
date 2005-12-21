<HTML>

<HEAD>
<TITLE>Ayuda de Web500gw</TITLE>
<META NAME="DC.Title"           LANG="es" CONTENT="Ayuda de la pasarela WWW/HTTP a X.500/LDAP">
<META NAME="DC.Creator"         LANG="es" CONTENT="Javier Massa, Javier.Massa@rediris.es">
<META NAME="DC.Creator"         LANG="es" CONTENT="x500@rediris.es">
<META NAME="DC.Subject"         LANG="es" CONTENT="x500, ldap, ayuda, web500gw, directorio">
<META NAME="DC.Description"     LANG="es" CONTENT="Ayuda de Web500gw: pasarela de World Wide Web/HTTP a X.500/LDAP que proporciona a clientes WWW acceso a directorios basados en el protocolo LDAP">
<META NAME="DC.Publisher"       LANG="es" CONTENT="CSIC/RedIRIS">
<META NAME="DC.Date"            CONTENT="1998-05-09">
<META NAME="DC.Language"        CONTENT="es">
</HEAD>

<BODY BGCOLOR="#FFFFFF" TEXT="#000000" LINK="#006600" VLINK="#003300" ALINK="#00FF00">

<!--
   - $Id: web500gw.help.es,v 1.3 2001/04/26 22:30:40 dekarl Exp $
  -->

<H2> Ayuda de la pasarela WWW/HTTP a X.500/LDAP</H2>


<STRONG><FONT COLOR="#006600">Web500gw</FONT></STRONG>
es una pasarela de World Wide Web/HTTP a X.500/LDAP (version 2.1).

Esta pasarela proporciona a clientes WWW acceso a directorios basados en el protocolo LDAP.
<p>Caracter�sticas de esta pasarela:

<p><STRONG>Lectura de entradas del Directorio</STRONG>
<ul>

	<p>Permite mostrar los atributos de una entrada. Dependiendo del tipo de atributo se mostrar�n de forma diferente: 
	<p>

	<UL>
		<LI type=disc>Los atributos que contienen un DN (nombre distintivo) (<TT>seeAlso, roleOccupant</TT>) son mostrados como enlaces hipertexto hacia esos DN.
		<LI>Los atributos que contienen una direcci�n de correo electr�nico son mostrados usando una acci�n especial (<TT>mailto:</TT>) que nos permite enviar un mensaje al pinchar sobre ese atributo.
		<LI>Los atributos que contienen una URI (<TT>labeledURI</TT>) son mostrados como enlaces hipertexto hacia esas URI.
		<LI>Las fotograf�as son mostradas en l�nea o mediante un enlace hipertexto para su descarga dependiendo del cliente que tengamos.
		<LI>Los datos en formato audio pueden ser recuperados v�a enlaces hipertextos.
	</UL>

	<p>Dependiendo de la configuraci�n existen "acciones especiales" para una entrada, por ejemplo enlaces ("Ver m�s atributos") o acciones de b�squeda ("listado de entradas hijas").

</ul>

<p><STRONG>Browse</STRONG>
<ul>
	Las entradas por debajo de una entrada pueden ser listadas como enlaces hipertexto, ordenadas por el tipo de objeto <tt>ObjectClass</tt>.
	Si se modifican los ficheros de configuraci�n o si se incorpora un par�metro al URL pueden mostrarse algunos atributos (ej: description) con el nombre de la entrada.
	Existe un bot�n de subida en la estructura jer�rquica que permite movernos hacia arriba en el �rbol de Directorio.
</ul>


<p><A NAME="search"><STRONG>B�squeda</STRONG></A><DD>
<ul>
	En la mayor�a de las p�ginas encontraremos un campo donde podremos escribir un t�rmino a buscar.
	En los niveles superiores del Directorio (<i>root</i> o nivel de pa�s) las b�squedas se asumen que son a un solo nivel (<i>one-level</i>).
	En los niveles inferiores (organizaciones y departamentos) las b�squedas por defecto se realizan en todo el sub�rbol que cuelga de la entrada en la que nos encontremos.
	Estos tipos de b�squedas son configurables por el administrador de web500gw.

	<p>Se soportan b�squedas de tipo amigable (UFN: <i>User Friendly Name</i>) mediante:
	<BLOCKQUOTE>
   	<TT>Nombre, Organzaci�n, Pa�s</TT>
	</BLOCKQUOTE>
	<p>Se pueden especificar complejos filtros de b�squeda mediante la inclusi�n de t�rminos en el URL pero a�n no se han implementado de modo interactivo.
</ul>

<p><STRONG>Modificaciones</STRONG>
<ul>
	Para modificar una entrada hay que conectarse al Directorio como un usuario autentificado. Hasta la fecha s�lo se soporta el modo de autentificaci�n simple (password).
	Es posible autentificarse como el Manager para modificar otras entradas.
	Despu�s de una conexi�n correcta se ofrece un formulario que contiene los datos de la entrada a modificar.
</ul>

Para m�s informaci�n consulte la <A HREF="http://web500gw.sourceforge.net/"><STRONG>documentaci�n en l�nea de Web500gw</STRONG></A>.
<P>
<HR size=1 noshade>
<P>

<STRONG><FONT COLOR="#006600">Web500gw</FONT></STRONG>
est� escrito por 
<A HREF="/cn=Frank%20Richter,ou=Rechenzentrum,o=Technische%20Universitaet%20Chemnitz,%20c=DE">
Frank Richter</A>, de la Technical University de Chemnitz, en Alemania.
<p>El c�digo est� basado en la implementaci�n de la pasarela Gopher-LDAP (go500gw) de Tim Howes, Netscape Communications Corp.
Muchas gracias a Mark Smith, Netscape Communications Corp., Rakesh Patel, Rutgers University y
<A HREF="/cn%3dHallvard%20B%20Furuseth%2c%20ou%3dUniversitetets%20Senter%20for%20Informasjonsteknologi%2c%20o%3dUniversitetet%20i%20Oslo%2c%20c%3dNO">
Hallvard B Furuseth</A>, Oslo University por sus contribuciones.
<p>Gracias a todos los que han participado con sus comentarios, trucos, preguntas y dem�s.

Si tiene comentarios, sugerencias o preguntas sobre esta pasarela, env�e un mensaje a
<A HREF="mailto:Frank.Richter@Hrz.TU-Chemnitz.DE">
Frank.Richter@Hrz.TU-Chemnitz.DE</A>.

<p><b>El autor no es responsable de los datos almacenados en el Directorio</b>
<P>
Para m�s informaci�n sobre Directorios, X.500 y LDAP :
<UL>
	<LI><A HREF="http://www.rediris.es/x500/">El Servicio de Directorio en RedIRIS, ES</A>. 
	<LI><A HREF="http://www.bath.ac.uk/~ccsap/Directory/x500.html">El Servicio de Directorio en la Universidad de Bath, UK</A>. 
	<LI><A HREF="http://www.nexor.co.uk/public/directory.html">Informaci�n sobre X.500 enNEXOR</A>
	<LI><A HREF="http://www.umich.edu/~dirsvcs/ldap/index.html">Informaci�n sobre LDAP en la Universidad de Michigan</A>.
</UL>

<P>
<HR>
<A HREF="/cn=Frank%20Richter,%20ou=Rechenzentrum,%20o=Technische%20Universitaet%20Chemnitz,%20c=DE"> Frank Richter</A>,
<A HREF="/o=Technische%20Universitaet%20Chemnitz,c=DE">
Technical University of Chemnitz</A>, 
<A HREF="/c=DE">Germany</A>.

</BODY>

</HTML>
