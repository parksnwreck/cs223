#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

enum state {FINDTRKPTSTART, FINDLAT, FINDLON, FINDELE, FINDTIME, FINDTRKPTEND};

int main()
{
  // the current state of the FSM
  enum state curr = FINDTRKPTSTART;

  // the first character in file
  int c = getchar();
  int lvlcnt = 0;

  while (c != EOF)
    {
      switch (curr)
	{
		case FINDTRKPTSTART:
		{
		  if (c == '<')
		    {
		      c = getchar();
			  if (c == 't' || c == 'T')
			    {
			      c = getchar();
			      if (c == 'r' || c == 'R')
			      	{
			      	c = getchar();
			      	if (c == 'k' || c == 'K')
				      {
				      	c = getchar();
						if (c == 'p' || c == 'P')
					      {
					      	c = getchar();
					      	if (c == 't' || c == 'T')
						      {
						      	c = getchar();
						      	if (isspace(c))
						      	{
							      	curr = FINDLAT;
							      	lvlcnt ++;
						      	}
						      }
					      }
				       }
			      	}
			    }
			}
		  else
		    {
		    	c = getchar();
		    }
		  break;
		}

		case FINDLAT:
	    {
	      if (isspace(c))
	      {
	      	while (isspace(c) && c!= EOF)
	      	{
	      		c = getchar();
	      	}
	      	if (c == 'l' || c == 'L')
		    {
			  c = getchar();
			  if (c == 'a' || c == 'A')
			    {
			      c = getchar();
			      if (c == 't' || c == 'T')
			      	{
			      	  c = getchar();
			      	  if (isspace(c) || c == '=')
			      	  {
			      	  	  c = getchar();
				      	  while (c != '"' && c != 39 && c!= EOF)
				      	  {
				      	  	c = getchar();
				      	  }
				      	  if (c == '"')
				      	  {
				      	  	c = getchar();
				      	  	while (c != '"' && c != EOF)
				      	  	{
								putchar (c);				      	    
					      	    c = getchar();
				      	  	}
				      	  	putchar (',');
				      	    c = getchar();
				      	  	curr = FINDLON;
				      	  }
				      	  if (c == 39)
				      	  {
				      	  	c = getchar();
				      	  	while (c != 39 && c!= EOF)
				      	  	{
								putchar (c);				      	    
					      	    c = getchar();
				      	  	}
				      	  	putchar (',');
				      	    c = getchar();
				      	  	curr = FINDLON;
				      	  }
			      	  }
			        }
		        }
		    }
	      }
		  else if (c == '"')
		  {
		    	c = getchar();
		    	while (c != '"' && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		  }
		  else if (c == 39)
		  {
		    	c = getchar();
		    	while (c != 39 && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		  }
		  else	
		  {
		    	c = getchar();
		  }
		  break;
  		}

		case FINDLON:
		{
		  if (isspace(c))
	      {
	      	  while (isspace(c) && c!= EOF)
	      	  {
	      		c = getchar();
	      	  }
			  if (c == 'l' || c == 'L')
			    {
				  c = getchar();
				  if (c == 'o' || c == 'O')
				    {
				      c = getchar();
				      if (c == 'n' || c == 'N')
				      	{
				      	  c = getchar();
				      	  if (isspace(c) || c == '=')
			      	      {	
					      	  c = getchar();
					      	  while (c != '"' && c != 39 && c!= EOF)
					      	  {
					      	  	c = getchar();
					      	  }

					      	  if (c == '"')
					      	  {
					      	  	c = getchar();
					      	  	while (c != '"' && c!= EOF)
					      	  	{
									putchar (c);				      	    
						      	    c = getchar();
					      	  	}
					      	  	putchar (',');
					      	    c = getchar();
					      	    while (c != '>' && c != EOF) 
					      	    {
					      	  		c = getchar();
					      	  	}
					      	  	c = getchar();
					      	  	curr = FINDELE;
					      	  }
				
					      	  if (c == 39)
					      	  {
					      	  	c = getchar();
					      	  	while (c != 39 && c!= EOF)
					      	  	{
									putchar (c);				      	    
						      	    c = getchar();
					      	  	}
					      	  	putchar (',');
					      	    c = getchar();
					      	    while (c != '>' && c != EOF) 
					      	    {
					      	  		c = getchar();
					      	  	}
					      	  	c = getchar();
					      	  	curr = FINDELE;
					      	  }
				      	  }  
				       }
			        }
			    }
		    }
		    else if (c == '"')
		    {
		    	c = getchar();
		    	while (c != '"' && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		    }
		    else if (c == 39)
		    {
		    	c = getchar();
		    	while (c != 39 && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		    }
		    else	
		    {
		    	c = getchar();
		    }
		    break;
		}

		case FINDELE:
		{
		  if (c == '<')
		    {
		      c = getchar();
		      if (c == '/')
		      {
		      	lvlcnt --;
		      	c = getchar();
		      	break;
		      }
		      else
		      {
		      	lvlcnt ++;
		      }
			  if ((c == 'e' || c == 'E') && lvlcnt == 2)
			    {
			      c = getchar();
			      if (c == 'l' || c == 'L')
			      	{
			      	c = getchar();
			      	if (c == 'e' || c == 'E')
				      {
				      	c = getchar();
				        while (c != '>' && c != EOF) 
			      	    {
			      	  		c = getchar();
			      	  	}
			      	  	c = getchar();
			      	  	while (c != '<' && c != EOF)
			      	  	{
			      	  		putchar(c);		      	  		
			      	  		c = getchar();
			      	  	}
			      	  	putchar(',');
			      	  	if (c == '<')
			      	  	{
			      	  		c = getchar();
						    if (c == '/')
						    {
							    lvlcnt --;
						    	c = getchar();
								if (c == 'e' || c == 'E')
						    	{
						      		c = getchar();
						      		if (c == 'l' || c == 'L')
						      		{
						      			c = getchar();
						      			if (c == 'e' || c == 'E')
						      			{
							      			while (c != '>' && c != EOF) 
								      	    {
								      	  		c = getchar();
								      	  	}
								      	  	c = getchar();
								      	  	curr = FINDTIME;
						      			}
						      		}
						      	}
						    }
						}
			      	  }
			        }
			    }
		  }
		  else if (c == '"')
		  {
		    	c = getchar();
		    	while (c != '"' && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		  }
		  else if (c == 39)
		  {
		    	c = getchar();
		    	while (c != 39 && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		  }
		  else
		    {
		    	c = getchar();
		    }
		  break;
		}

		case FINDTIME:
		{
		   if (c == '<')
		    {
		      c = getchar();
		      if (c == '/')
		      {
		      	lvlcnt --;
		      	c = getchar();
		      	break;
		      }
		      else
		      {
		      	lvlcnt ++;
		      }
		      if ((c == 't' || c == 'T') && lvlcnt == 2)
		      {
		      	  c = getchar();
				  if (c == 'i' || c == 'I')
				    {
				      c = getchar();
				      if (c == 'm' || c == 'M')
				      	{
				      	c = getchar();
				      	if (c == 'e' || c == 'E')
					      {
					      	c = getchar();
					        while (c != '>' && c != EOF) 
				      	    {
				      	  		c = getchar();
				      	  	}
				      	  	c = getchar();
				      	  	while (c != '<' && c != EOF)
				      	  	{
				      	  		if (c == 44)
				      	  		{
				      	  			printf("&comma;");
				      	  		}
				      	  		else
				      	  		{
									putchar(c);	
				      	  		}
				      	  		c = getchar();	      	  		
				      	  	}
				      	  	putchar('\n');
				      	  	if (c == '<')
				      	  	{
				      	  		c = getchar();
							    if (c == '/')
							    {
							    	lvlcnt --;
							    	c = getchar();
							    	if (c == 't' || c == 'T')
							    	{
										c = getchar();
								    	if (c == 'i' || c == 'I')
								    	{
								      		c = getchar();
								      		if (c == 'm' || c == 'M')
								      		{
								      			c = getchar();
								      			if (c == 'e' || c == 'E')
								      			{
								      				c = getchar();
									      			while (c != '>' && c != EOF) 
										      	    {
										      	  		c = getchar();
										      	  	}
													c = getchar();
										      	  	curr = FINDTRKPTEND;
								      			}
								      		}
								      	}
							    	}
							    }
							}
				      	  }
				        }
				    }
				}
		  }
		  else if (c == '"')
		  {
		    	c = getchar();
		    	while (c != '"' && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		  }
		  else if (c == 39)
		  {
		    	c = getchar();
		    	while (c != 39 && c != EOF)
		    	{
		    		c = getchar();
		    	}
		    	c = getchar();
		  }

		  else
		  {
		    	c = getchar();
		  }
		  break;
		}

		case FINDTRKPTEND:
		{
		  if (c == '<')
		    {
		      c = getchar();
		      if (c == '/')
		        {
		          lvlcnt --;
		          c = getchar();
				  if (c == 't' || c == 'T')
				    {
				      c = getchar();
				      if (c == 'r' || c == 'R')
				      	{
				      	c = getchar();
				      	if (c == 'k' || c == 'K')
					      {
					      	c = getchar();
							if (c == 'p' || c == 'P')
						      {
						      	c = getchar();
						      	if (c == 't' || c == 'T')
							      {
					      			while (c != '>' && c != EOF) 
						      	    {
						      	  		c = getchar();
						      	  	}
									c = getchar();
									curr = FINDTRKPTSTART;
							      }
						      }
					       }
				      	}
				    }
		        }
			}
		  else
		  {
		  	c = getchar();
		  }
		  break;
		}
	 }
  }
}

