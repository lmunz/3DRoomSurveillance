using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace NavigationDrawerPopUpMenu2
{
    public partial class UserControlCreate : UserControl
    {
        public UserControlCreate()
        {
            InitializeComponent();
            var videos = GetVideos();
            if (videos.Count > 0)
                ListViewProducts.ItemsSource = videos;
        }
        private List<Videos> GetVideos()
        {
            return new List<Videos>()
      {
        new Videos("/Assets/1.jpg"),
        new Videos("/Assets/2.jpg"),
        new Videos("/Assets/3.jpg"),
        new Videos("/Assets/4.jpg"),
        new Videos("/Assets/1.jpg"),
        new Videos("/Assets/2.jpg"),
        new Videos("/Assets/3.jpg"),
        new Videos("/Assets/4.jpg"),
        new Videos("/Assets/1.jpg"),
        new Videos("/Assets/2.jpg"),
        new Videos("/Assets/3.jpg"),
        new Videos("/Assets/4.jpg"),
        new Videos("/Assets/2.jpg"),
        new Videos("/Assets/3.jpg"),
        new Videos("/Assets/4.jpg"),
        new Videos("/Assets/1.jpg"),
        new Videos("/Assets/2.jpg"),
        new Videos("/Assets/3.jpg"),
        new Videos("/Assets/4.jpg"),
         new Videos("/Assets/2.jpg"),
        new Videos("/Assets/3.jpg"),
        new Videos("/Assets/4.jpg"),
        new Videos("/Assets/1.jpg"),
        new Videos("/Assets/2.jpg"),
        new Videos("/Assets/3.jpg"),
        new Videos("/Assets/4.jpg")

      };
        }
    }
}
