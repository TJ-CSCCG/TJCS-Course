<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template math="/">
	<html>
	<body>
		<center>
		<table border="1" width="300" >
		<tr>
			<td align="center">name</td>
			<td align="center">sex</td>
		</tr>
		<xsl:for-each select="message/persion">
		
		<tr>
			<td align="center"><xsl:value-of select="name" /></td>
			<xsl:choose>
			  <xsl:when test="sex[value()$eq$'male']">
			     <td align="center">男</td>
			  </xsl:when>
			  <xsl:when test="sex[value()$eq$'female']">
			     <td align="center">女</td>
			  </xsl:when>
			  <xsl:otherwise>
          		    <td align="center" bgcolor='yellow'>未知</td>
         		  </xsl:otherwise>
			</xsl:choose>
		</tr>
		</xsl:for-each>
		</table>
		</center>
	</body>
	</html>
</xsl:template>
</xsl:stylesheet>