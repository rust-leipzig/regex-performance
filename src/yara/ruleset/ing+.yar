rule ing
{
  strings:
    $re = /[a-zA-Z]+ing/
  condition:
    $re
}
