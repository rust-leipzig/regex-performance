rule ing
{
  strings:
    $re = /\s[a-zA-Z]{0,12}ing\s/
  condition:
    $re
}
