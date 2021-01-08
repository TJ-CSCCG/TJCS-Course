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
		<xsl:if test="sex[.='male']">
		<tr>
			<td align="center"><xsl:value-of select="name" /></td>
			<td align="center"><xsl:value-of select="sex" /></td>
		</tr>
		</xsl:if>
		</xsl:for-each>
		</table>
		</center>
	</body>
	</html>
</xsl:template>
</xsl:stylesheet>