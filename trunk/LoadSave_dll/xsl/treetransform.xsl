<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format">

<xsl:template match="/"> 
 <HTML> 
  <xsl:apply-templates/>
 </HTML> 
</xsl:template> 

<xsl:template match="save">
 <head>
  <title>Esgaroth-Engine Save File viewer -
   <xsl:apply-templates select="save_name"/>
  </title>
  <script language="javascript" src="tree.js"></script>  
 </head>
 <body>
 <style type="text/css">
  a:link {text-decoration: none}
  a:hover {text-decoration: underline}
 </style>
  <font face="Courier New, sans-serif" size="2">
   <table border='0' cellpadding='0' cellspacing='0'><tr><td>
    <xsl:apply-templates select="object"/>
   </td></tr></table>
  </font>
 </body>
</xsl:template> 

<xsl:template match="save_name"> 
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="object"> 
<!-- build the id for the java script -->
 <a id="toggle_{record}" href="javascript:Toggle('{record}');">
  <img src='collapsed.bmp' hspace='0' vspace='0' border='0'/>
  <b>
   <span style="color:#0000FF;">
    <xsl:apply-templates select="name"/>
   </span>
  </b>
  <span style="color:#000000;">
   <xsl:text>&#032;</xsl:text>
   <xsl:apply-templates select="../name"/> 
  </span>
  <b>
   <span style="color:#990000;">
    (<xsl:apply-templates select="id"/>)
   </span>
  </b>
 </a>
 <div id="{record}" style="display: none; margin-left: 11px;">
  <xsl:apply-templates select="var"/>
 </div>
 <xsl:if test="../save_name">
  <br/>
 </xsl:if>
</xsl:template>

<xsl:template match="id">
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="record">
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="name">
 <xsl:apply-templates/>
</xsl:template> 

<!-- quite a messy bunch of shit, but does as it should -->
<xsl:template match="var">
 <br/>
 <xsl:choose>
  <xsl:when test="type_modifier = 'array'">
   <a id="toggle_{record}" href="javascript:Toggle('{record}');">
    <img src='collapsed.bmp' hspace='0' vspace='0' border='0'/>
    <b><span style="color:#0000FF;">
     <xsl:apply-templates select="type"/>[]
    </span></b>
    <span style="color:#000000;">
     <xsl:text>&#032;</xsl:text>
     <xsl:apply-templates select="name"/>
    </span>
   </a>
  <!-- build the div-line for the java script -->
   <div id="{record}" style="display: none; margin-left: 11px;">
    <table border="1">
	 <tr>
      <xsl:apply-templates select="arr_column_indices"/>
     </tr>
	  <xsl:apply-templates select="arr_line"/>
    </table>
   </div>
  
  </xsl:when>
  <xsl:otherwise>
   <xsl:if test="value">
    <b><span style="color:#0000FF;">
     <xsl:apply-templates select="type"/>
    </span></b>
    <xsl:text>&#032;</xsl:text>
    <xsl:apply-templates select="name"/>
    =
    <xsl:apply-templates select="value"/>
   </xsl:if>
   <xsl:apply-templates select="object"/>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

<xsl:template match="type">
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="value">
 '<xsl:apply-templates/>'
</xsl:template> 

<!-- array templates -->

<xsl:template match="index">
 <b><span style="color:#000000;">
  <xsl:apply-templates/> 
 </span></b>
</xsl:template> 

<xsl:template match="arr_column_indices">
 <td/><!-- the first div is empty -->
 <xsl:for-each select="value">
  <td>
   <b><span style="color:#000000;">
    <xsl:apply-templates/>
   </span></b> 
  </td>
 </xsl:for-each>
 
</xsl:template>

<xsl:template match="arr_line">
 <tr>
  <td> <xsl:apply-templates select="index"/> </td>
  <xsl:choose>
   <xsl:when test="var">
    <xsl:for-each select="var">
     <td>
	  <!-- mad hax: this code was copied from the match="var" and decreased by the amount of the name tag code-->
      <xsl:choose>
       <xsl:when test="type_modifier = 'array'">
        <a id="toggle_{record}" href="javascript:Toggle('{record}');">
         <img src='collapsed.bmp' hspace='0' vspace='0' border='0'/>
         <b><span style="color:#0000FF;">
          <xsl:apply-templates select="type"/>[]
         </span></b>
        </a>
        <!-- build the div-line for the java script -->
        <div id="{record}" style="display: none; margin-left: 11px;">
         <table border="1">
	      <tr>
           <xsl:apply-templates select="arr_column_indices"/>
          </tr>
	      <xsl:apply-templates select="arr_line"/>
         </table>
        </div>
       </xsl:when>
       <xsl:otherwise>
        <xsl:apply-templates select="object"/>
       </xsl:otherwise>
      </xsl:choose>
     </td>
    </xsl:for-each>
   </xsl:when>
   <xsl:otherwise>
    <xsl:for-each select="value">
     <td>
      <xsl:apply-templates/>
     </td>
    </xsl:for-each>
   </xsl:otherwise>
  </xsl:choose>
 </tr>
</xsl:template>

<xsl:template match="text()">
 <xsl:value-of select="."/>
</xsl:template>

</xsl:stylesheet>

