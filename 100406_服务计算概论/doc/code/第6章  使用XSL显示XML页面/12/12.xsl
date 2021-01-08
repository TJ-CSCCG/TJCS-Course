<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
  <xsl:template math="/">
	<html>
	<body>
		<center>
		<table border="1" width="300" >
		<tr>
			<td align="center">name</td>
			<td align="center">age</td>
		</tr>
		<xsl:for-each select="message/persion" order-by="name;age" >
		<tr>
			<td align="center"><xsl:value-of select="name" /></td>
			<td align="center"><xsl:value-of select="age" /></td>
		</tr>
		</xsl:for-each>
		</table>
		</center>
	</body>
	</html>
  </xsl:template>
</xsl:stylesheet>