function Toggle(item)
{
	obj=document.getElementById(item);
	visible=(obj.style.display!="none")
	key=document.getElementById("toggle_" + item);
	if (visible)
	{
		obj.style.display="none";
		key.innerHTML=key.innerHTML.replace("open", "collapsed");
	}
	else
	{
		obj.style.display="block";
		key.innerHTML=key.innerHTML.replace("collapsed", "open");
	}
}