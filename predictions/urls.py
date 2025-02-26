from django.urls import path
from . import views
from django.http import HttpResponse


def debug(request):
    return HttpResponse("Charts URL is recognized correctly!")

urlpatterns = [
    path('', views.home, name='home'),  # Home page
    path('predict/', views.predict, name='predict'),  # Prediction endpoint
    path('charts/', views.charts, name='charts'),
    path('live-data/', views.live_data, name='live-data'),
    path('current-status/', views.current_status, name='current-status'),  # Current status page
    path('debug/', debug, name='debug')
]
