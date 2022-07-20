rule twain
{
  strings:
    $str = "Twain"
  condition:
    $str
}
